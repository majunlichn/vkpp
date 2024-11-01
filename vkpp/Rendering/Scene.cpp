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
    rad::Ref<Image> image = m_context->GetDevice()->CreateImage2DTexture(VK_FORMAT_R8G8B8A8_UNORM, 256, 256, 1);
    rad::Ref<ImageView> imageView = image->CreateDefaultView();
    m_images.push_back(std::move(image));
    m_imageViews.push_back(std::move(imageView));
}

Scene::~Scene()
{
}

bool Scene::Upload()
{
    for (const auto& mesh : m_meshes)
    {
        mesh->Upload();
    }
    return true;
}

} // namespace vkpp
