#include <vkpp/Core/Pipeline.h>
#include <vkpp/Core/PhysicalDevice.h>
#include <vkpp/Core/Device.h>
#include <vkpp/Core/RenderPass.h>
#include <rad/IO/File.h>

namespace vkpp
{

ShaderModule::ShaderModule(
    rad::Ref<Device> device,
    const VkShaderModuleCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateShaderModule(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

ShaderModule::~ShaderModule()
{
    m_device->GetFunctionTable()->
        vkDestroyShaderModule(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}

PipelineLayout::PipelineLayout(rad::Ref<Device> device,
    const VkPipelineLayoutCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreatePipelineLayout(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

PipelineLayout::~PipelineLayout()
{
    m_device->GetFunctionTable()->
        vkDestroyPipelineLayout(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}


Pipeline::Pipeline(rad::Ref<Device> device, VkPipelineBindPoint bindPoint) :
    m_device(std::move(device)),
    m_bindPoint(bindPoint)
{
}

Pipeline::~Pipeline()
{
    m_device->GetFunctionTable()->
        vkDestroyPipeline(m_device->GetHandle(), m_handle, nullptr);
}

VkPipelineBindPoint Pipeline::GetBindPoint() const
{
    return m_bindPoint;
}

bool Pipeline::SaveCacheToFile(VkPipelineCache cache, std::string_view filePath)
{
    size_t dataSize = 0;

    VK_CHECK(m_device->GetFunctionTable()->
        vkGetPipelineCacheData(m_device->GetHandle(), cache, &dataSize, nullptr));

    if (dataSize > 0)
    {
        std::vector<uint8_t> cacheData;
        cacheData.resize(dataSize);
        VK_CHECK(m_device->GetFunctionTable()->
            vkGetPipelineCacheData(m_device->GetHandle(), cache, &dataSize, cacheData.data()));
        rad::File file;
        if (file.Open(filePath, "wb"))
        {
            file.Write(cacheData.data(), dataSize);
            file.Close();
            return true;
        }
    }
    return false;
}

GraphicsPipeline::GraphicsPipeline(rad::Ref<Device> device,
    const VkGraphicsPipelineCreateInfo& createInfo) :
    Pipeline(std::move(device), VK_PIPELINE_BIND_POINT_GRAPHICS)
{
    VK_CHECK(m_device->GetFunctionTable()->vkCreateGraphicsPipelines(
        m_device->GetHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_handle));
}

GraphicsPipeline::~GraphicsPipeline()
{
}

ComputePipeline::ComputePipeline(rad::Ref<Device> device,
    const VkComputePipelineCreateInfo& createInfo) :
    Pipeline(std::move(device), VK_PIPELINE_BIND_POINT_COMPUTE)
{
    VK_CHECK(m_device->GetFunctionTable()->vkCreateComputePipelines(
        m_device->GetHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_handle));
}

ComputePipeline::~ComputePipeline()
{
}

GraphicsPipelineCreateInfo::GraphicsPipelineCreateInfo(Device* device) :
    m_device(device)
{
    VkStencilOpState stencilOpState = {};
    stencilOpState.failOp = VK_STENCIL_OP_KEEP;
    stencilOpState.passOp = VK_STENCIL_OP_KEEP;
    stencilOpState.depthFailOp;
    stencilOpState.compareOp = VK_COMPARE_OP_ALWAYS;
    stencilOpState.compareMask;
    stencilOpState.writeMask;
    stencilOpState.reference;

    m_depthStencil.front = stencilOpState;
    m_depthStencil.back = stencilOpState;
}

GraphicsPipelineCreateInfo::~GraphicsPipelineCreateInfo()
{
}

const VkGraphicsPipelineCreateInfo& GraphicsPipelineCreateInfo::Setup()
{
    PhysicalDevice* physicalDevice = m_device->GetPhysicalDevice();
    m_pipelineInfo = {};
    m_pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    m_pipelineInfo.pNext = nullptr;
    m_pipelineInfo.flags = 0;

    VK_STRUCTURE_CHAIN_BEGIN(m_pipelineInfo);

    m_shaderStageInfos.clear();
    m_specializationInfos.clear();
    m_shaderStageInfos.reserve(m_shaderStages.size());
    for (const auto& shaderStage : m_shaderStages)
    {
        VkPipelineShaderStageCreateInfo shaderStageInfo = {};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.pNext = nullptr;
        shaderStageInfo.flags = 0;
        shaderStageInfo.stage = shaderStage.m_stage;
        shaderStageInfo.module = shaderStage.shader->GetHandle();
        shaderStageInfo.pName = "main";
        if (shaderStage.specialization)
        {
            m_specializationInfos.push_back(shaderStage.specialization->GetInfo());
            shaderStageInfo.pSpecializationInfo = &m_specializationInfos.back();
        }
        else
        {
            shaderStageInfo.pSpecializationInfo = nullptr;
        }
        m_shaderStageInfos.push_back(shaderStageInfo);
    }

    m_pipelineInfo.stageCount = static_cast<uint32_t>(m_shaderStageInfos.size());
    m_pipelineInfo.pStages = m_shaderStageInfos.data();

    m_vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_vertexInputInfo.pNext = nullptr;
    m_vertexInputInfo.flags = 0;
    m_vertexInputInfo.vertexBindingDescriptionCount =
        static_cast<uint32_t>(m_vertexInput.bindings.size());
    m_vertexInputInfo.pVertexBindingDescriptions =
        m_vertexInput.bindings.data();
    m_vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(m_vertexInput.attributes.size());
    m_vertexInputInfo.pVertexAttributeDescriptions =
        m_vertexInput.attributes.data();
    m_pipelineInfo.pVertexInputState = &m_vertexInputInfo;

    m_inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_inputAssemblyInfo.pNext = nullptr;
    m_inputAssemblyInfo.flags = 0;
    m_inputAssemblyInfo.topology = m_inputAssembly.topology;
    m_inputAssemblyInfo.primitiveRestartEnable = m_inputAssembly.primitiveRestartEnable;
    m_pipelineInfo.pInputAssemblyState = &m_inputAssemblyInfo;

    VkShaderStageFlags pipelineStageFlags = 0;
    for (const auto& shaderStage : m_shaderStages)
    {
        pipelineStageFlags |= shaderStage.m_stage;
    }

    if (rad::HasBits<uint32_t>(pipelineStageFlags,
        VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))
    {
        m_tessellationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        m_tessellationInfo.pNext = nullptr;
        m_tessellationInfo.flags = 0;
        m_tessellationInfo.patchControlPoints = m_tessellation.patchControlPoints;
        m_pipelineInfo.pTessellationState = &m_tessellationInfo;
    }

    m_viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_viewportInfo.pNext = nullptr;
    m_viewportInfo.flags = 0;
    m_viewportInfo.viewportCount = m_viewportCount;
    m_viewportInfo.pViewports = nullptr; // will be set dynamically
    m_viewportInfo.scissorCount = m_scissorCount;
    m_viewportInfo.pScissors = nullptr; // will be set dynamically
    m_pipelineInfo.pViewportState = &m_viewportInfo;

    m_rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_rasterizationInfo.pNext = nullptr;
    m_rasterizationInfo.flags = 0; // is reserved for future use.
    m_rasterizationInfo.depthClampEnable = m_rasterization.depthClampEnable;
    m_rasterizationInfo.rasterizerDiscardEnable = m_rasterization.rasterizerDiscardEnable;
    m_rasterizationInfo.polygonMode = m_rasterization.polygonMode;
    m_rasterizationInfo.cullMode = m_rasterization.cullMode;
    m_rasterizationInfo.frontFace = m_rasterization.frontFace;
    m_rasterizationInfo.depthBiasEnable = m_rasterization.depthBiasEnable;
    m_rasterizationInfo.depthBiasConstantFactor = m_rasterization.depthBiasConstantFactor;
    m_rasterizationInfo.depthBiasClamp = m_rasterization.depthBiasClamp;
    m_rasterizationInfo.depthBiasSlopeFactor = m_rasterization.depthBiasSlopeFactor;
    m_rasterizationInfo.lineWidth = m_rasterization.lineWidth;
    m_pipelineInfo.pRasterizationState = &m_rasterizationInfo;

    m_multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    m_multisampleInfo.pNext = nullptr;
    m_multisampleInfo.flags = 0;
    m_multisampleInfo.rasterizationSamples = m_multisample.rasterizationSamples;
    m_multisampleInfo.sampleShadingEnable = m_multisample.sampleShadingEnable;
    m_multisampleInfo.minSampleShading = m_multisample.minSampleShading;
    m_multisampleInfo.pSampleMask = &m_multisample.sampleMask;
    m_multisampleInfo.alphaToCoverageEnable = m_multisample.alphaToCoverageEnable;
    m_multisampleInfo.alphaToOneEnable = m_multisample.alphaToOneEnable;
    m_pipelineInfo.pMultisampleState = &m_multisampleInfo;

    m_depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_depthStencilInfo.pNext = nullptr;
    m_depthStencilInfo.flags = 0;
    m_depthStencilInfo.depthTestEnable = m_depthStencil.depthTestEnable;
    m_depthStencilInfo.depthWriteEnable = m_depthStencil.depthWriteEnable;
    m_depthStencilInfo.depthCompareOp = m_depthStencil.depthCompareOp;
    m_depthStencilInfo.depthBoundsTestEnable = m_depthStencil.depthBoundsTestEnable;
    m_depthStencilInfo.stencilTestEnable = m_depthStencil.stencilTestEnable;
    m_depthStencilInfo.front = m_depthStencil.front;
    m_depthStencilInfo.back = m_depthStencil.back;
    m_depthStencilInfo.minDepthBounds = m_depthStencil.minDepthBounds;
    m_depthStencilInfo.maxDepthBounds = m_depthStencil.maxDepthBounds;
    m_pipelineInfo.pDepthStencilState = &m_depthStencilInfo;

    m_colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_colorBlendInfo.pNext = nullptr;
    m_colorBlendInfo.flags = 0;
    m_colorBlendInfo.logicOpEnable = m_colorBlend.logicOpEnable;
    m_colorBlendInfo.logicOp = m_colorBlend.logicOp;
    m_colorBlendInfo.attachmentCount = static_cast<uint32_t>(m_colorBlend.attachments.size());
    m_colorBlendInfo.pAttachments = m_colorBlend.attachments.data();
    m_colorBlendInfo.blendConstants[0] = m_colorBlend.blendConstants[0];
    m_colorBlendInfo.blendConstants[1] = m_colorBlend.blendConstants[1];
    m_colorBlendInfo.blendConstants[2] = m_colorBlend.blendConstants[2];
    m_colorBlendInfo.blendConstants[3] = m_colorBlend.blendConstants[3];
    m_pipelineInfo.pColorBlendState = &m_colorBlendInfo;

    m_dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    m_dynamicInfo.pNext = nullptr;
    m_dynamicInfo.flags = 0;
    m_dynamicInfo.dynamicStateCount = static_cast<uint32_t>(m_dynamicStates.size());
    m_dynamicInfo.pDynamicStates = m_dynamicStates.data();
    m_pipelineInfo.pDynamicState = &m_dynamicInfo;

    m_pipelineInfo.layout = m_layout ? m_layout->GetHandle() : VK_NULL_HANDLE;
    if (m_renderPass)
    {
        m_pipelineInfo.renderPass = m_renderPass->GetHandle();
    }
    else if (physicalDevice->m_vk13Features.dynamicRendering)
    {
        if (m_colorFormats.size() > 0 &&
            (m_renderingInfo.sType == VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR))
        {
            VK_STRUCTURE_CHAIN_ADD(m_pipelineInfo, m_renderingInfo);
        }
    }
    m_pipelineInfo.subpass = m_subpass;
    m_pipelineInfo.basePipelineHandle = m_basePipeline ?
        m_basePipeline->GetHandle() : VK_NULL_HANDLE;
    m_pipelineInfo.basePipelineIndex = m_basePipelineIndex;

    VK_STRUCTURE_CHAIN_END(m_pipelineInfo);
    return m_pipelineInfo;
}

void GraphicsPipelineCreateInfo::AddVertexBindingPacked(
    uint32_t binding, rad::Span<VkVertexInputAttrib> attribs, VkVertexInputRate inputRate)
{
    VkVertexInputBindingDescription bindingDesc = {};
    bindingDesc.binding = binding;
    bindingDesc.stride = 0;
    uint32_t location = 0;
    uint32_t offset = 0;
    for (const VkVertexInputAttrib& attrib : attribs)
    {
        VkVertexInputAttributeDescription attribDesc = {};
        attribDesc.location = attrib.location;
        attribDesc.binding = binding;
        attribDesc.format = attrib.format;
        attribDesc.offset = offset;
        m_vertexInput.attributes.push_back(attribDesc);
        location += 1;
        offset += vkuFormatElementSize(attrib.format);
    }
    bindingDesc.stride = offset;
    bindingDesc.inputRate = inputRate;
    m_vertexInput.bindings.push_back(bindingDesc);
}

void GraphicsPipelineCreateInfo::DisableColorBlend(uint32_t attachCount)
{
    m_colorBlend.attachments.resize(attachCount);
    for (auto& attachment : m_colorBlend.attachments)
    {
        attachment = {};
        attachment.blendEnable = VK_FALSE;
        attachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
    }
}

void GraphicsPipelineCreateInfo::SetRenderingInfo(
    rad::Span<VkFormat> colorFormats, VkFormat depthStencilFormat)
{
    m_renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    m_colorFormats = colorFormats;
    m_renderingInfo.colorAttachmentCount = uint32_t(m_colorFormats.size());
    m_renderingInfo.pColorAttachmentFormats = m_colorFormats.data();
    m_renderingInfo.depthAttachmentFormat = depthStencilFormat;
    m_renderingInfo.stencilAttachmentFormat = depthStencilFormat;
}

ComputePipelineCreateInfo::ComputePipelineCreateInfo(Device* device) :
    m_device(device)
{
}

ComputePipelineCreateInfo::~ComputePipelineCreateInfo()
{
}

const VkComputePipelineCreateInfo& ComputePipelineCreateInfo::Setup()
{
    m_pipelineInfo = {};
    m_pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    m_pipelineInfo.pNext = nullptr;
    m_pipelineInfo.flags = 0;
    m_pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_pipelineInfo.stage.pNext = nullptr;
    m_pipelineInfo.stage.flags = 0;
    m_pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    m_pipelineInfo.stage.module = m_shaderModule->GetHandle();
    m_pipelineInfo.stage.pName = "main";
    if (m_shaderSpecialization)
    {
        m_specializationInfo = m_shaderSpecialization->GetInfo();
        m_pipelineInfo.stage.pSpecializationInfo = &m_specializationInfo;
    }
    else
    {
        m_pipelineInfo.stage.pSpecializationInfo = nullptr;
    }
    m_pipelineInfo.layout = m_layout ?
        m_layout->GetHandle() : VK_NULL_HANDLE;
    m_pipelineInfo.basePipelineHandle = m_basePipeline ?
        m_basePipeline->GetHandle() : VK_NULL_HANDLE;
    m_pipelineInfo.basePipelineIndex = m_basePipelineIndex;
    return m_pipelineInfo;
}

} // namespace vkpp
