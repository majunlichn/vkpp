#pragma once

#include <vkpp/Core/Context.h>
#include <vkpp/Core/Math.h>

#include <rad/Core/Flags.h>

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

enum class VertexAttribute : uint32_t
{
    Position = 1 << 0,
    Normal = 1 << 1,
    Tangent = 1 << 2,
    Color = 1 << 3,
    UV0 = 1 << 4,
    UV1 = 1 << 5,
    UV2 = 1 << 6,
    UV3 = 1 << 7,
};

using VertexFormat = rad::Flags32<VertexAttribute>;

uint32_t GetVertexStride(VertexFormat format);

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

    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec4> m_tangents;
    std::vector<glm::vec3> m_bitangents;
    std::vector<glm::vec4> m_colors;
    std::vector<std::vector<glm::vec2>> m_uvChannels;
    std::vector<uint32_t> m_indices;

    std::vector<Bone> m_bones;

    AABB m_aabb;

    rad::Ref<Material> m_material;
    uint32_t m_materialIndex = 0; // cache the index in scene->m_materials?

    VertexFormat m_vertexFormat = VertexAttribute::Position;
    rad::Ref<Buffer> m_vertexBuffer;
    VkDeviceSize m_vertexBufferOffset = 0;
    VkDeviceSize m_vertexBufferSize = 0;
    rad::Ref<Buffer> m_indexBuffer;
    VkDeviceSize m_indexBufferOffset = 0;
    VkDeviceSize m_indexBufferSize = 0;

}; // class Mesh

} // namespace vkpp
