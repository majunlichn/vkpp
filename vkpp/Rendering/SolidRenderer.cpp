#include <vkpp/Rendering/SolidRenderer.h>
#include <vkpp/Core/ShaderCompiler.h>
#include <vkpp/Scene/SceneNode.h>
#include <vkpp/Scene/Mesh.h>
#include <vkpp/Scene/Material.h>
#include <vkpp/Scene/Light.h>
#include <vkpp/Scene/Camera.h>
#include <rad/Container/SmallVector.h>
#include <rad/IO/File.h>
#include <rad/IO/FileSystem.h>

namespace vkpp
{

SolidRenderer::SolidRenderer(rad::Ref<Context> context) :
    m_context(std::move(context))
{
}

SolidRenderer::~SolidRenderer()
{
    m_context->WaitIdle();
}

bool SolidRenderer::Init()
{
    Device* device = m_context->GetDevice();
    Queue* queue = m_context->GetQueue();
    m_samplers.resize(4);
    m_samplers[0] = device->CreatSamplerNearest();
    m_samplers[1] = device->CreatSamplerLinear(VK_SAMPLER_ADDRESS_MODE_REPEAT, 4.0f);
    m_samplers[2] = device->CreatSamplerLinear(VK_SAMPLER_ADDRESS_MODE_REPEAT, 8.0f);
    m_samplers[3] = device->CreatSamplerLinear(VK_SAMPLER_ADDRESS_MODE_REPEAT, 16.0f);

    const VkExtent2D& resolution = m_context->m_resolution;
    Resize(resolution.width, resolution.height);

    m_cmdBuffers.resize(m_context->m_swapchainImageCount);
    m_cmdPool = device->CreateCommandPool(queue->GetQueueFamily(),
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    for (size_t i = 0; i < m_cmdBuffers.size(); ++i)
    {
        m_cmdBuffers[i] = m_cmdPool->Allocate();
    }

    return true;
}

bool SolidRenderer::LoadScene(Scene* scene)
{
    m_scene = scene;

    Device* device = m_context->GetDevice();
    const auto& deviceProps = device->GetPhysicalDevice()->m_properties;
    const auto& swapchainImageCount = m_context->m_swapchainImageCount;

    m_uniformBuffers.resize(swapchainImageCount);
    m_uniformData.resize(swapchainImageCount);
    VkDeviceSize minOffsetAlign = deviceProps.limits.minUniformBufferOffsetAlignment;
    VkDeviceSize frameInfoStride = sizeof(FrameInfo);
    VkDeviceSize meshInfoStride = sizeof(MeshInfo);
    if (minOffsetAlign > 0)
    {
        frameInfoStride = rad::RoundUpToMultiple(frameInfoStride, minOffsetAlign);
        meshInfoStride = rad::RoundUpToMultiple(meshInfoStride, minOffsetAlign);
    }
    for (uint32_t i = 0; i < m_uniformBuffers.size(); ++i)
    {
        // created as persistent mapped:
        m_uniformBuffers[i] = device->CreateUniformBuffer(
            frameInfoStride + scene->m_meshes.size() * meshInfoStride, true);
        m_uniformData[i] = (uint8_t*)m_uniformBuffers[i]->GetMappedAddr();
    }

    m_frameDescSetLayout = device->CreateDescriptorSetLayout(
        { // binding, type, count, stageFlags, samplers
            { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr }, // renderTarget
            { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr }, // overlay
        }
        );

    rad::SmallVector<VkSampler, 4> samplerHandles;
    samplerHandles.resize(m_samplers.size());
    for (size_t i = 0; i < samplerHandles.size(); ++i)
    {
        samplerHandles[i] = m_samplers[i]->GetHandle();
    }
    m_sceneDescSetLayout = device->CreateDescriptorSetLayout(
        { // binding, type, count, stageFlags, samplers
            { 0, VK_DESCRIPTOR_TYPE_SAMPLER, 4, VK_SHADER_STAGE_ALL_GRAPHICS, samplerHandles.data() },
            { 1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, static_cast<uint32_t>(scene->m_image2Ds.size()), VK_SHADER_STAGE_ALL_GRAPHICS, nullptr},
        }
        );
    m_pipelineLayout = device->CreatePipelineLayout(
        { m_frameDescSetLayout.get(), m_sceneDescSetLayout.get() });
    GraphicsPipelineCreateInfo pipelineInfo(device);
    rad::Ref<ShaderCompiler> shaderCompiler = RAD_NEW ShaderCompiler();
    auto vertBinary = shaderCompiler->CompileGLSLFromFile(VK_SHADER_STAGE_VERTEX_BIT,
        "Rendering/Solid.vert",
        "main", {});
    auto fragBinary = shaderCompiler->CompileGLSLFromFile(VK_SHADER_STAGE_FRAGMENT_BIT,
        "Rendering/Solid.frag",
        "main", {});
    rad::Ref<ShaderModule> vert = device->CreateShaderModule(vertBinary);
    rad::Ref<ShaderModule> frag = device->CreateShaderModule(fragBinary);
    pipelineInfo.m_shaderStages =
    {
        { VK_SHADER_STAGE_VERTEX_BIT, vert, nullptr },
        { VK_SHADER_STAGE_FRAGMENT_BIT, frag, nullptr },
    };
    pipelineInfo.AddVertexBindingPacked(
        0,  // binding
        {
        VertexInputAttrib{.location = 0, .format = VK_FORMAT_R32G32B32_SFLOAT },    // Position
        VertexInputAttrib{.location = 1, .format = VK_FORMAT_R32G32B32_SFLOAT },    // Normal
        VertexInputAttrib{.location = 2, .format = VK_FORMAT_R32G32B32A32_SFLOAT }, // Tangent
        VertexInputAttrib{.location = 3, .format = VK_FORMAT_R32G32_SFLOAT },       // UV
        }
        );
    pipelineInfo.m_inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInfo.m_depthStencil.depthTestEnable = VK_TRUE;
    pipelineInfo.m_depthStencil.depthWriteEnable = VK_TRUE;
    pipelineInfo.m_depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineInfo.SetColorBlendDisabled(1);
    pipelineInfo.m_layout = m_pipelineLayout;
    pipelineInfo.SetRenderingInfo(m_context->m_colorFormat, m_context->m_depthStencilFormat);
    m_pipelines[RenderType::TriangleListTextured] =
        device->CreateGraphicsPipeline(pipelineInfo.Setup());

    m_descPool = device->CreateDescriptorPool(swapchainImageCount + 1,
        {
            VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, swapchainImageCount },
            VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_SAMPLER, 4 },
            VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, static_cast<uint32_t>(scene->m_image2Ds.size()) },
        });
    m_frameDescSets.resize(swapchainImageCount);
    for (size_t i = 0; i < m_frameDescSets.size(); ++i)
    {
        m_frameDescSets[i] = m_descPool->Allocate(m_frameDescSetLayout.get());
    }
    m_sceneDescSet = m_descPool->Allocate(m_sceneDescSetLayout.get());

    SetupResourceBindings();

    return true;
}

bool SolidRenderer::SetupResourceBindings()
{
    for (size_t i = 0; i < m_frameDescSets.size(); ++i)
    {
        m_frameDescSets[i]->UpdateBuffers(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            VkDescriptorBufferInfo{ m_uniformBuffers[i]->GetHandle(), 0, sizeof(FrameInfo) });
        m_frameDescSets[i]->UpdateBuffers(1, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            VkDescriptorBufferInfo{ m_uniformBuffers[i]->GetHandle(), 0, sizeof(MeshInfo) });
    }
    std::vector<VkImageLayout> imageLayouts(m_scene->m_image2Ds.size(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_sceneDescSet->UpdateImages(1, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        m_scene->m_image2DViews, imageLayouts);
    return true;
}

void SolidRenderer::Render()
{
    if (!m_scene || !m_renderTarget)
    {
        return;
    }

    m_uniformDataOffset = 0;

    FrameInfo frameInfo = {};
    frameInfo.viewProj = m_scene->m_camera->GetViewProjectionMatrix();
    m_frameInfoOffset = WriteUniforms(&frameInfo, sizeof(frameInfo));

    CommandBuffer* cmdBuffer = m_cmdBuffers[m_cmdBufferIndex].get();
    cmdBuffer->Begin();
    if (m_renderTarget->GetCurrentLayout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        cmdBuffer->TransitLayoutFromCurrent(m_renderTarget.get(),
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
    VkClearColorValue colorValue = {};
    VkClearDepthStencilValue depthStencilValue = {};
    depthStencilValue.depth = 1.0f;
    depthStencilValue.stencil = 0;
    cmdBuffer->BeginRendering(
        m_renderTargetView.get(), &colorValue,
        m_depthStencilView.get(), &depthStencilValue);

    SceneNode* node = m_scene->m_root.get();
    Render(cmdBuffer, node);

    cmdBuffer->EndRendering();
    cmdBuffer->End();
    m_cmdBufferIndex += 1;
    m_cmdBufferIndex %= m_cmdBuffers.size();

    m_context->GetQueue()->Submit(cmdBuffer);
}

void SolidRenderer::Render(CommandBuffer* cmdBuffer, SceneNode* node)
{
    for (size_t i = 0; i < node->m_meshes.size(); ++i)
    {
        Mesh* mesh = node->m_meshes[i].get();
        MeshInfo meshInfo = {};
        meshInfo.toWorld = node->m_transformToWorld;
        meshInfo.wireframeColor = glm::vec3(1.0f, 1.0f, 1.0f);
        meshInfo.shadeWireframe = true;
        Material* material = mesh->m_material.get();
        if (TextureInfo* textureInfo = material->m_baseColorTexture.get())
        {
            meshInfo.baseColorTextureIndex = textureInfo->imageIndex;
            meshInfo.baseColorSamplerIndex = 3; // sampler index
            meshInfo.baseColorUVIndex = textureInfo->uvIndex;
            meshInfo.baseColorLodBias = 0.0f;
        }

        uint32_t meshInfoOffset = WriteUniforms(&meshInfo, sizeof(meshInfo));
        Pipeline* pipeline = m_pipelines[mesh->m_renderType].get();
        cmdBuffer->BindPipeline(pipeline);
        cmdBuffer->BindDescriptorSets(
            pipeline, m_pipelineLayout.get(),
            0, { m_frameDescSets[m_cmdBufferIndex].get(), m_sceneDescSet.get() },
            { m_frameInfoOffset, meshInfoOffset }
        );
        cmdBuffer->BindVertexBuffers(
            0, mesh->m_vertexBuffer.get(), mesh->m_vertexBufferOffset);
        cmdBuffer->BindIndexBuffer(
            mesh->m_indexBuffer.get(), mesh->m_indexBufferOffset,
            VK_INDEX_TYPE_UINT32);

        const VkExtent2D& resolution = m_context->m_resolution;
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = float(resolution.width);
        viewport.height = float(resolution.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = resolution;
        cmdBuffer->SetViewports(viewport);
        cmdBuffer->SetScissors(scissor);

        cmdBuffer->DrawIndexed(mesh->GetIndexCount(), 1, 0, 0, 0);
    }

    for (const rad::Ref<SceneNode>& child : node->m_children)
    {
        Render(cmdBuffer, child.get());
    }
}

void SolidRenderer::Resize(uint32_t width, uint32_t height)
{
    Device* device = m_context->GetDevice();
    m_renderTarget = device->CreateImage2DRenderTarget(
        m_context->m_colorFormat, width, height, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_renderTargetView = m_renderTarget->CreateDefaultView();
    m_depthStencil = device->CreateImage2DDepthStencil(
        m_context->m_depthStencilFormat, width, height);
    m_depthStencilView = m_depthStencil->CreateDefaultView();
}

uint32_t SolidRenderer::WriteUniforms(void* data, size_t sizeInBytes)
{
    uint32_t offset = static_cast<uint32_t>(m_uniformDataOffset);
    memcpy(m_uniformData[m_cmdBufferIndex] + m_uniformDataOffset, data, sizeInBytes);
    const auto& props = m_context->GetDevice()->GetPhysicalDevice()->m_properties;
    m_uniformDataOffset += rad::RoundUpToMultiple<size_t>(sizeInBytes,
        static_cast<size_t>(props.limits.minUniformBufferOffsetAlignment));
    return offset;
}

} // namespace vkpp
