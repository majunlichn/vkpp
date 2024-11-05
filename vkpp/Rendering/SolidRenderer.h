#pragma once

#include <vkpp/Rendering/Scene.h>
#include <vkpp/Rendering/Mesh.h>

namespace vkpp
{

class SolidRenderer : public rad::RefCounted<SolidRenderer>
{
public:
    SolidRenderer(rad::Ref<Context> context);
    ~SolidRenderer();

    struct FrameInfo
    {
        glm::mat4 viewProj;
    };

    struct MeshInfo
    {
        glm::mat4 toWorld;
        glm::uvec4 baseColorTextureIndex;
    };

    bool Init();
    bool LoadScene(Scene* scene);
    bool SetupResourceBindings();
    void Render();
    void Render(CommandBuffer* cmdBuffer, SceneNode* node);
    void Resize(uint32_t width, uint32_t height);

    rad::Ref<Context> m_context;
    rad::Ref<Image> m_renderTarget;
    rad::Ref<ImageView> m_renderTargetView;
    rad::Ref<Image> m_depthStencil;
    rad::Ref<ImageView> m_depthStencilView;

    std::vector<rad::Ref<Buffer>> m_uniformBuffers;
    std::vector<uint8_t*> m_uniformData;
    VkDeviceSize m_uniformDataOffset = 0;
    uint32_t WriteUniforms(void* data, size_t sizeInBytes);
    uint32_t m_frameInfoOffset = 0;
    rad::Ref<DescriptorSetLayout> m_frameDescSetLayout;
    rad::Ref<DescriptorSetLayout> m_sceneDescSetLayout;
    rad::Ref<PipelineLayout> m_pipelineLayout;
    std::map<RenderType, rad::Ref<Pipeline>> m_pipelines;
    rad::Ref<DescriptorPool> m_descPool;
    std::vector<rad::Ref<DescriptorSet>> m_frameDescSets;
    rad::Ref<DescriptorSet> m_sceneDescSet;

    std::vector<rad::Ref<Sampler>> m_samplers;
    Scene* m_scene = nullptr;

    rad::Ref<CommandPool> m_cmdPool;
    std::vector<rad::Ref<CommandBuffer>> m_cmdBuffers;
    size_t m_cmdBufferIndex = 0;

}; // class SolidRenderer

} // namespace vkpp
