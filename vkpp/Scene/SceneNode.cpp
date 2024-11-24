#include <vkpp/Scene/SceneNode.h>
#include <vkpp/Scene/Scene.h>
#include <vkpp/Scene/Mesh.h>
#include <vkpp/Scene/Material.h>
#include <vkpp/Scene/Light.h>
#include <vkpp/Scene/Camera.h>

namespace vkpp
{

SceneNode::SceneNode(Scene* scene, SceneNode* parent) :
    m_scene(scene),
    m_parent(parent)
{
    m_transform = glm::identity<glm::mat4>();
    m_transformToWorld = glm::identity<glm::mat4>();
}

SceneNode::~SceneNode()
{
}

SceneNode* SceneNode::AddChild()
{
    auto& child = m_children.emplace_back(RAD_NEW SceneNode(m_scene, this));
    return child.get();
}

SceneNode* SceneNode::FindChild(std::string_view name, bool recursive)
{
    for (auto& child : m_children)
    {
        if (rad::StrEqual(child->m_name, name))
        {
            return child.get();
        }
    }
    if (recursive)
    {
        for (auto& child : m_children)
        {
            if (SceneNode* node = child->FindChild(name, recursive))
            {
                return node;
            }
        }
    }
    return nullptr;
}

AABB SceneNode::GetBoundingBox() const
{
    AABB nodeBox;
    if (m_meshes.size() > 0)
    {
        for (size_t i = 0; i < m_meshes.size(); ++i)
        {
            const AABB& meshBox = m_meshes[i]->m_aabb;
            nodeBox = Unite(nodeBox, meshBox);
        }
        nodeBox = Transform(nodeBox, m_transformToWorld);
    }

    for (size_t i = 0; i < m_children.size(); ++i)
    {
        const AABB& childBox = m_children[i]->GetBoundingBox();
        nodeBox = Unite(nodeBox, childBox);
    }
    return nodeBox;
}

} // namespace vkpp
