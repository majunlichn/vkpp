#include <vkpp/Rendering/Mesh.h>
#include <vkpp/Rendering/Scene.h>
#include <vkpp/Rendering/Material.h>

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
    }
    return 0;
}

bool Mesh::Upload()
{
    Device* device = m_scene->m_context->GetDevice();
    rad::Ref<Buffer> stagingBuffer = device->CreateStagingBuffer(
        m_positions.size() * GetVertexStride(m_renderType));

    if ((m_renderType == RenderType::PointList) ||
        (m_renderType == RenderType::LineList))
    {
        uint8_t* pStaging = (uint8_t*)stagingBuffer->MapMemory();
        memcpy(pStaging, m_positions.data(), m_positions.size() * sizeof(glm::vec3));
        stagingBuffer->UnmapMemory();
    }
    else if (m_renderType == RenderType::TriangleList)
    {
        uint8_t* pStaging = (uint8_t*)stagingBuffer->MapMemory();
        for (size_t i = 0; i < m_positions.size(); ++i)
        {
            memcpy(pStaging, &m_positions[i], sizeof(glm::vec3));
            pStaging += sizeof(glm::vec3);
            memcpy(pStaging, &m_normals[i], sizeof(glm::vec3));
            pStaging += sizeof(glm::vec3);
        }
        stagingBuffer->UnmapMemory();
    }
    else if ((m_renderType == RenderType::PointListColored) ||
        (m_renderType == RenderType::LineListColored))
    {
        uint8_t* pStaging = (uint8_t*)stagingBuffer->MapMemory();
        for (size_t i = 0; i < m_positions.size(); ++i)
        {
            memcpy(pStaging, &m_positions[i], sizeof(glm::vec3));
            pStaging += sizeof(glm::vec3);
            memcpy(pStaging, &m_colors[i], sizeof(glm::vec4));
            pStaging += sizeof(glm::vec4);
        }
        stagingBuffer->UnmapMemory();
    }
    else if (m_renderType == RenderType::TriangleListColored)
    {
        uint8_t* pStaging = (uint8_t*)stagingBuffer->MapMemory();
        for (size_t i = 0; i < m_positions.size(); ++i)
        {
            memcpy(pStaging, &m_positions[i], sizeof(glm::vec3));
            pStaging += sizeof(glm::vec3);
            memcpy(pStaging, &m_normals[i], sizeof(glm::vec3));
            pStaging += sizeof(glm::vec3);
            memcpy(pStaging, &m_colors[i], sizeof(glm::vec4));
            pStaging += sizeof(glm::vec4);
        }
        stagingBuffer->UnmapMemory();
    }
    else if (m_renderType == RenderType::TriangleListTextured)
    {
        uint8_t* pStaging = (uint8_t*)stagingBuffer->MapMemory();
        for (size_t i = 0; i < m_positions.size(); ++i)
        {
            memcpy(pStaging, &m_positions[i], sizeof(glm::vec3));
            pStaging += sizeof(glm::vec3);
            memcpy(pStaging, &m_normals[i], sizeof(glm::vec3));
            pStaging += sizeof(glm::vec3);
            memcpy(pStaging, &m_tangents[i], sizeof(glm::vec4));
            pStaging += sizeof(glm::vec4);
            memcpy(pStaging, &m_uvChannels[0][i], sizeof(glm::vec2));
            pStaging += sizeof(glm::vec2);
        }
        stagingBuffer->UnmapMemory();
    }

    return true;
}

} // namespace vkpp
