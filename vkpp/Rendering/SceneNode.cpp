#include <vkpp/Rendering/SceneNode.h>
#include <vkpp/Rendering/Scene.h>
#include <vkpp/Rendering/Mesh.h>
#include <vkpp/Rendering/Material.h>
#include <vkpp/Rendering/Light.h>
#include <vkpp/Rendering/Camera.h>

namespace vkpp
{

SceneNode::SceneNode(Scene* scene, SceneNode* parent) :
    m_scene(scene),
    m_parent(parent)
{
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

} // namespace vkpp
