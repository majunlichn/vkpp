#include <vkpp/Rendering/Mesh.h>
#include <vkpp/Rendering/Material.h>

namespace vkpp
{

uint32_t GetVertexStride(VertexFormat format)
{
    uint32_t stride = 0;
    if (format & VertexAttribute::Position)
    {
        stride += sizeof(glm::vec3);
    }
    if (format & VertexAttribute::Normal)
    {
        stride += sizeof(glm::vec3);
    }
    if (format & VertexAttribute::Tangent)
    {
        stride += sizeof(glm::vec4);
    }
    if (format & VertexAttribute::Color)
    {
        stride += sizeof(glm::vec4);
    }
    if (format & VertexAttribute::UV0)
    {
        stride += sizeof(glm::vec2);
    }
    if (format & VertexAttribute::UV1)
    {
        stride += sizeof(glm::vec2);
    }
    if (format & VertexAttribute::UV2)
    {
        stride += sizeof(glm::vec2);
    }
    if (format & VertexAttribute::UV3)
    {
        stride += sizeof(glm::vec2);
    }
    return stride;
}

Mesh::Mesh(Scene* scene) :
    m_scene(scene)
{
}

Mesh::~Mesh()
{
}

} // namespace vkpp
