#include <vkpp/Rendering/Scene.h>
#include <vkpp/Rendering/SceneNode.h>
#include <vkpp/Rendering/Mesh.h>
#include <vkpp/Rendering/Material.h>
#include <vkpp/Rendering/Light.h>
#include <vkpp/Rendering/Camera.h>

namespace vkpp
{

Scene::Scene(rad::Ref<Context> context) :
    m_context(std::move(context))
{
    m_root = RAD_NEW SceneNode(this, nullptr);
}

Scene::~Scene()
{
}

} // namespace vkpp
