#include <vkpp/Scene/Mesh.h>
#include <vkpp/Scene/Scene.h>
#include <vkpp/Scene/Material.h>

namespace vkpp
{

Mesh::Mesh(Scene* scene) :
    m_scene(scene)
{
}

Mesh::~Mesh()
{
}

uint32_t Mesh::GetVertexStride(RenderType renderType)
{
    switch (renderType)
    {
    case RenderType::PointList:
        return static_cast<uint32_t>(sizeof(glm::vec3));
    case RenderType::PointListColored:
        return static_cast<uint32_t>(sizeof(glm::vec3) + sizeof(glm::vec4));
    case RenderType::LineList:
        return static_cast<uint32_t>(sizeof(glm::vec3));
    case RenderType::LineListColored:
        return static_cast<uint32_t>(sizeof(glm::vec3) + sizeof(glm::vec4));
    case RenderType::TriangleList:
        return static_cast<uint32_t>(sizeof(glm::vec3) + sizeof(glm::vec3));
    case RenderType::TriangleListColored:
        return static_cast<uint32_t>(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec4));
    case RenderType::TriangleListTextured:
        return static_cast<uint32_t>(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec4) + sizeof(glm::vec2));
    case RenderType::TriangleListTexturedSkinning:
        return static_cast<uint32_t>(
            sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec4) + sizeof(glm::vec2)
            + sizeof(glm::uvec4) + sizeof(glm::vec4));
    }
    return 0;
}

bool Mesh::Upload()
{
    Context* context = m_scene->m_context.get();
    Device* device = context->GetDevice();
    if (!m_vertexBuffer)
    {
        m_vertexBufferSize = m_positions.size() * GetVertexStride(m_renderType);
        m_vertexBuffer = device->CreateVertexBuffer(m_vertexBufferSize);
    }
    if (!m_indices.empty() && !m_indexBuffer)
    {
        m_indexBufferSize = m_indices.size() * sizeof(uint32_t);
        m_indexBuffer = device->CreateIndexBuffer(m_indexBufferSize);
    }

    rad::Ref<Buffer> vertexStagingBuffer = device->CreateStagingBuffer(m_vertexBufferSize);
    rad::Ref<Buffer> indexStagingBuffer;
    if (m_indexBufferSize > 0)
    {
        indexStagingBuffer = device->CreateStagingBuffer(m_indexBufferSize);
    }

    uint8_t* pVertexStaging = (uint8_t*)vertexStagingBuffer->MapMemory();

    if ((m_renderType == RenderType::PointList) ||
        (m_renderType == RenderType::LineList))
    {
        memcpy(pVertexStaging, m_positions.data(), m_positions.size() * sizeof(glm::vec3));
    }
    else if (m_renderType == RenderType::TriangleList)
    {
        for (size_t i = 0; i < m_positions.size(); ++i)
        {
            memcpy(pVertexStaging, &m_positions[i], sizeof(glm::vec3));
            pVertexStaging += sizeof(glm::vec3);
            memcpy(pVertexStaging, &m_normals[i], sizeof(glm::vec3));
            pVertexStaging += sizeof(glm::vec3);
        }
    }
    else if ((m_renderType == RenderType::PointListColored) ||
        (m_renderType == RenderType::LineListColored))
    {
        for (size_t i = 0; i < m_positions.size(); ++i)
        {
            memcpy(pVertexStaging, &m_positions[i], sizeof(glm::vec3));
            pVertexStaging += sizeof(glm::vec3);
            memcpy(pVertexStaging, &m_colors[i], sizeof(glm::vec4));
            pVertexStaging += sizeof(glm::vec4);
        }
    }
    else if (m_renderType == RenderType::TriangleListColored)
    {
        for (size_t i = 0; i < m_positions.size(); ++i)
        {
            memcpy(pVertexStaging, &m_positions[i], sizeof(glm::vec3));
            pVertexStaging += sizeof(glm::vec3);
            memcpy(pVertexStaging, &m_normals[i], sizeof(glm::vec3));
            pVertexStaging += sizeof(glm::vec3);
            memcpy(pVertexStaging, &m_colors[i], sizeof(glm::vec4));
            pVertexStaging += sizeof(glm::vec4);
        }
    }
    else if (m_renderType == RenderType::TriangleListTextured)
    {
        for (size_t i = 0; i < m_positions.size(); ++i)
        {
            memcpy(pVertexStaging, &m_positions[i], sizeof(glm::vec3));
            pVertexStaging += sizeof(glm::vec3);
            memcpy(pVertexStaging, &m_normals[i], sizeof(glm::vec3));
            pVertexStaging += sizeof(glm::vec3);
            memcpy(pVertexStaging, &m_tangents[i], sizeof(glm::vec4));
            pVertexStaging += sizeof(glm::vec4);
            memcpy(pVertexStaging, &m_uvChannels[0][i], sizeof(glm::vec2));
            pVertexStaging += sizeof(glm::vec2);
        }
    }

    vertexStagingBuffer->UnmapMemory();

    if (m_indexBufferSize > 0)
    {
        uint8_t* pIndexStaging = (uint8_t*)indexStagingBuffer->MapMemory();
        memcpy(pIndexStaging, m_indices.data(), m_indexBufferSize);
        indexStagingBuffer->UnmapMemory();
    }

    QueueFamily queueFamily = QueueFamilyUniversal;
    rad::Ref<CommandPool> cmdPool =
        device->CreateCommandPool(queueFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

    rad::Ref<CommandBuffer> cmdBuffer = cmdPool->Allocate();
    cmdBuffer->Begin();
    VkBufferCopy copyVertexRegion = {};
    copyVertexRegion.srcOffset = 0;
    copyVertexRegion.dstOffset = m_vertexBufferOffset;
    copyVertexRegion.size = m_vertexBufferSize;
    cmdBuffer->CopyBuffer(vertexStagingBuffer.get(), m_vertexBuffer.get(), copyVertexRegion);
    VkBufferCopy copyIndexRegion = {};
    copyIndexRegion.srcOffset = 0;
    copyIndexRegion.dstOffset = m_indexBufferOffset;
    copyIndexRegion.size = m_indexBufferSize;
    cmdBuffer->CopyBuffer(indexStagingBuffer.get(), m_indexBuffer.get(), copyIndexRegion);
    cmdBuffer->End();
    context->GetQueue(queueFamily)->SubmitAndWait(cmdBuffer.get());

    return true;
}

} // namespace vkpp
