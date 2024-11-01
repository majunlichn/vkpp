#pragma once

#include <vkpp/Core/Context.h>
#include <vkpp/Core/Math.h>

namespace vkpp
{

class Scene;
class Material;

enum class PrimitiveType : uint32_t
{
    Point,
    Line,
    Triangle,
};

enum class RenderType : uint32_t
{
    PointList,          // position
    PointListColored,   // position + color
    LineList,           // position
    LineListColored,    // position + color
    TriangleList,           // position + normal
    TriangleListColored,    // position + normal + color
    TriangleListTextured,   // position + normal + tangent + uv
};

struct VertexWeight
{
    uint32_t index;
    float weight;
};

struct Bone
{
    std::string name;
    std::vector<VertexWeight> weights;
    // Matrix that transforms from mesh space to bone space in bind pose.
    glm::mat4 inverseBindMatrix;
};

class Mesh : public rad::RefCounted<Mesh>
{
public:
    Mesh(Scene* scene);
    ~Mesh();

    Scene* m_scene;
    std::string m_name;
    PrimitiveType m_primitiveType = PrimitiveType::Point;

    uint32_t GetVertexCount() const { return static_cast<uint32_t>(m_positions.size()); }
    uint32_t GetIndexCount() const { return static_cast<uint32_t>(m_indices.size()); }

    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec4> m_tangents;
    std::vector<glm::vec3> m_bitangents;
    std::vector<glm::vec4> m_colors;
    std::vector<std::vector<glm::vec2>> m_uvChannels;
    std::vector<uint32_t> m_indices;

    std::vector<Bone> m_bones;

    AABB m_aabb;

    RenderType m_renderType = RenderType::PointList;
    static uint32_t GetVertexStride(RenderType renderType);

    // Upload data to GPU according to renderType.
    bool Upload();

    rad::Ref<Buffer> m_vertexBuffer;
    VkDeviceSize m_vertexBufferOffset = 0;
    VkDeviceSize m_vertexBufferSize = 0;
    rad::Ref<Buffer> m_indexBuffer;
    VkDeviceSize m_indexBufferOffset = 0;
    VkDeviceSize m_indexBufferSize = 0;

    rad::Ref<Material> m_material;
    uint32_t m_materialIndex = 0; // cache the index in scene->m_materials?

}; // class Mesh

} // namespace vkpp
