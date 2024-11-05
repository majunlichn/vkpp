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
    m_camera = RAD_NEW Camera(this);
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

AABB Scene::GetBoundingBox() const
{
    return m_root->GetBoundingBox();
}

void Scene::SetCameraFrontView()
{
    AABB sceneBox = GetBoundingBox();
    glm::vec3 center = (sceneBox.m_min + sceneBox.m_max) / 2.0f;
    float w = sceneBox.m_max.x - sceneBox.m_min.x;
    float h = sceneBox.m_max.y - sceneBox.m_min.y;
    float d = sceneBox.m_max.z - sceneBox.m_min.z;
    d = h / 2.0f / std::tanf(m_camera->m_yfov / 2.0f);
    m_camera->m_position = center - glm::vec3(0.0f, 0.0f, center.z - d);
    m_camera->m_lookAt = center;
    m_camera->m_up = glm::vec3(0, 1, 0);
}

} // namespace vkpp
