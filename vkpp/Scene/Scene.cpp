#include <vkpp/Scene/Scene.h>
#include <vkpp/Scene/SceneNode.h>
#include <vkpp/Scene/Mesh.h>
#include <vkpp/Scene/Material.h>
#include <vkpp/Scene/Light.h>
#include <vkpp/Scene/Camera.h>
#include <vkpp/Core/Texture.h>

namespace vkpp
{

Scene::Scene(rad::Ref<Context> context) :
    m_context(std::move(context))
{
    m_root = RAD_NEW SceneNode(this, nullptr);
    m_camera = RAD_NEW Camera(this);
    rad::Ref<Image> image = m_context->GetDevice()->CreateImage2DTexture(VK_FORMAT_R8G8B8A8_UNORM, 256, 256, 1);
    rad::Ref<ImageView> imageView = image->CreateDefaultView();
    m_image2Ds.push_back(std::move(image));
    m_image2DViews.push_back(std::move(imageView));
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
    m_image2Ds.resize(1);
    m_image2DViews.resize(1);
    for (const auto& material : m_materials)
    {
        material->Upload();
    }
    return true;
}

uint32_t Scene::UploadImage2DFromFile(std::string_view fileName, bool genMipmaps)
{
    uint32_t index = GetImage2DIndex(fileName);
    if (index == 0)
    {
        rad::Ref<Image> texture = CreateImage2DFromFile(m_context.get(), fileName, genMipmaps);
        m_image2Ds.push_back(texture);
        m_image2DViews.push_back(texture->CreateDefaultView());
        assert(m_image2Ds.size() == m_image2DViews.size());
        index = static_cast<uint32_t>(m_image2Ds.size() - 1);
        m_imageNameIndexMap[std::string(fileName)] = index;
    }
    return index;
}

uint32_t Scene::GetImage2DIndex(std::string_view path)
{
    auto iter = m_imageNameIndexMap.find(path);
    if (iter != m_imageNameIndexMap.end())
    {
        return static_cast<uint32_t>(iter->second);
    }
    return 0;
}

Image* Scene::GetImage2D(std::string_view path)
{
    auto iter = m_imageNameIndexMap.find(path);
    if (iter != m_imageNameIndexMap.end())
    {
        return m_image2Ds[iter->second].get();
    }
    return nullptr;
}

AABB Scene::GetBoundingBox() const
{
    return m_root->GetBoundingBox();
}

void Scene::SetCameraFrontView()
{
    AABB sceneBox = GetBoundingBox();
    glm::vec3 center = sceneBox.GetCenter();
    float w = sceneBox.m_max.x - sceneBox.m_min.x;
    float h = sceneBox.m_max.y - sceneBox.m_min.y;
    float d = sceneBox.m_max.z - sceneBox.m_min.z;
    d = h / 2.0f / std::tanf(m_camera->m_yfov / 2.0f);
    m_camera->m_position = center - glm::vec3(0.0f, 0.0f, center.z - d);
    m_camera->m_lookAt = center;
    m_camera->m_up = glm::vec3(0, 1, 0);
}

} // namespace vkpp
