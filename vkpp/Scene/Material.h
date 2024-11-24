#pragma once

#include <vkpp/Core/Context.h>
#include <vkpp/Core/Math.h>

namespace vkpp
{

class Scene;

struct TextureInfo : public rad::RefCounted<TextureInfo>
{
    std::string fileName;
    // All images are managed in scene->m_image2Ds (same fileNames refers to one image, with same index).
    uint32_t imageIndex;
    uint32_t uvIndex;
    VkSamplerAddressMode addressModeU;
    VkSamplerAddressMode addressModeV;
};

enum class AlphaMode
{
    Opaque,
    Mask,
    Blend,
};

class Material : public rad::RefCounted<Material>
{
public:
    Material(Scene* scene);
    ~Material();

    bool Upload();

    Scene* m_scene;
    std::string m_name;
    glm::vec4 m_baseColor = { 1, 1, 1, 1 };
    rad::Ref<TextureInfo> m_baseColorTexture;
    float m_metallic = 1.0f;
    float m_roughness = 1.0f;
    rad::Ref<TextureInfo> m_metallicRoughnessTexture;
    rad::Ref<TextureInfo> m_normalTexture;
    float m_normalScale = 1.0f;
    rad::Ref<TextureInfo> m_ambientTexture;
    float m_ambientStrength = 1.0f;
    glm::vec3 m_emissiveColor = { 0, 0, 0 };
    rad::Ref<TextureInfo> m_emissiveTexture;

    AlphaMode m_alphaMode = AlphaMode::Opaque;
    float m_alphaCutoff = 0.5f;

    bool m_doubleSided = false;

}; // class Material

} // namespace vkpp
