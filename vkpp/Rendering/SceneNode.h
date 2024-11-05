#pragma once

#include <vkpp/Core/Context.h>
#include <vkpp/Core/Math.h>

namespace vkpp
{

class Scene;
class Mesh;
class Material;
class Light;
class Camera;

class SceneNode : public rad::RefCounted<SceneNode>
{
public:
    SceneNode(Scene* scene, SceneNode* parent);
    ~SceneNode();

    SceneNode* GetParent() { return m_parent; }
    const SceneNode* GetParent() const { return m_parent; }
    SceneNode* AddChild();
    SceneNode* GetChild(size_t index) { return m_children[index].get(); }
    const SceneNode* GetChild(size_t index) const { return m_children[index].get(); }
    SceneNode* FindChild(std::string_view name, bool recursive = true);

    Scene* m_scene = nullptr;
    SceneNode* m_parent = nullptr;
    std::vector<rad::Ref<SceneNode>> m_children;

    std::string m_name;

    std::vector<rad::Ref<Mesh>> m_meshes;
    std::vector<rad::Ref<Material>> m_materials;
    std::vector<rad::Ref<Light>> m_lights;
    std::vector<rad::Ref<Camera>> m_cameras;

    glm::mat4 m_transform = glm::identity<glm::mat4>();
    glm::mat4 m_transformToWorld = glm::identity<glm::mat4>();

    AABB GetBoundingBox() const;

}; // class SceneNode

} // namespace vkpp
