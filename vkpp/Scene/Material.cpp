#include <vkpp/Scene/Material.h>
#include <vkpp/Scene/Scene.h>

namespace vkpp
{

Material::Material(Scene* scene) :
    m_scene(scene)
{
}

Material::~Material()
{
}

bool Material::Upload()
{
    if (m_baseColorTexture)
    {
        m_baseColorTexture->imageIndex = m_scene->UploadImage2DFromFile(
            m_baseColorTexture->fileName, true
        );
    }
    if (m_metallicRoughnessTexture)
    {
        m_metallicRoughnessTexture->imageIndex = m_scene->UploadImage2DFromFile(
            m_metallicRoughnessTexture->fileName, true
        );
    }
    if (m_normalTexture)
    {
        m_normalTexture->imageIndex = m_scene->UploadImage2DFromFile(
            m_normalTexture->fileName, true
        );
    }
    if (m_ambientTexture)
    {
        m_ambientTexture->imageIndex = m_scene->UploadImage2DFromFile(
            m_ambientTexture->fileName, true
        );
    }
    if (m_emissiveTexture)
    {
        m_emissiveTexture->imageIndex = m_scene->UploadImage2DFromFile(
            m_emissiveTexture->fileName, true
        );
    }
    return true;
}

} // namespace vkpp
