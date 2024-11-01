#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class ShaderModule : public rad::RefCounted<ShaderModule>
{
public:
    ShaderModule(
        rad::Ref<Device> device, const VkShaderModuleCreateInfo& createInfo);
    ~ShaderModule();
    VKPP_DISABLE_COPY_AND_MOVE(ShaderModule);

    VkShaderModule GetHandle() const { return m_handle; }

private:
    rad::Ref<Device> m_device;
    VkShaderModule m_handle;

}; // class ShaderModule

class PipelineLayout : public rad::RefCounted<PipelineLayout>
{
public:
    PipelineLayout(
        rad::Ref<Device> device,
        const VkPipelineLayoutCreateInfo& createInfo);
    ~PipelineLayout();
    VKPP_DISABLE_COPY_AND_MOVE(PipelineLayout);

    VkPipelineLayout GetHandle() const { return m_handle; }

private:
    rad::Ref<Device> m_device;
    VkPipelineLayout m_handle = VK_NULL_HANDLE;

}; // class PipelineLayout

class Pipeline : public rad::RefCounted<Pipeline>
{
public:
    Pipeline(rad::Ref<Device> device, VkPipelineBindPoint bindPoint);
    ~Pipeline();

    operator VkPipeline() const { return m_handle; }
    VkPipeline GetHandle() const { return m_handle; }

    VkPipelineBindPoint GetBindPoint() const;
    bool SaveCacheToFile(VkPipelineCache cache, std::string_view filePath);

protected:
    rad::Ref<Device>            m_device;
    VkPipeline                  m_handle = VK_NULL_HANDLE;
    VkPipelineBindPoint         m_bindPoint;

}; // class Pipeline

class GraphicsPipeline : public Pipeline
{
public:
    GraphicsPipeline(
        rad::Ref<Device> device,
        const VkGraphicsPipelineCreateInfo& createInfo);
    ~GraphicsPipeline();

}; // class GraphicsPipeline

class ComputePipeline : public Pipeline
{
public:
    ComputePipeline(
        rad::Ref<Device> device,
        const VkComputePipelineCreateInfo& createInfo);
    ~ComputePipeline();

}; // class ComputePipeline

struct SpecializationInfo : public rad::RefCounted<SpecializationInfo>
{
    std::vector<VkSpecializationMapEntry> m_mapEntries;
    std::vector<uint8_t> m_data;

    template<typename T, typename = std::enable_if_t<std::is_trivial_v<T>>>
    void Add(uint32_t constantID, const T& t)
    {
        size_t offset = m_data.size();
        m_data.resize(offset + sizeof(T));
        memcpy(m_data.data() + offset, &t, sizeof(T));
        VkSpecializationMapEntry entry = {};
        entry.constantID = constantID;
        entry.offset = offset;
        entry.size = sizeof(T);
        m_mapEntries.push_back(entry);
    }

    void Clear()
    {
        m_mapEntries.clear();
        m_data.clear();
    }

    VkSpecializationInfo GetInfo()
    {
        VkSpecializationInfo info = {};
        info.mapEntryCount = static_cast<uint32_t>(m_mapEntries.size());
        info.pMapEntries = m_mapEntries.data();
        info.dataSize = m_data.size();
        info.pData = m_data.data();
        return info;
    }

}; // struct SpecializationInfo

struct VertexInputAttrib
{
    uint32_t location;
    VkFormat format;
};

struct GraphicsPipelineCreateInfo : public rad::RefCounted<GraphicsPipelineCreateInfo>
{
private:
    Device* m_device;
    VkGraphicsPipelineCreateInfo m_pipelineInfo = {};
    std::vector<VkPipelineShaderStageCreateInfo> m_shaderStageInfos;
    std::vector<VkSpecializationInfo> m_specializationInfos;
    VkPipelineVertexInputStateCreateInfo m_vertexInputInfo = {};
    VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyInfo = {};
    VkPipelineTessellationStateCreateInfo m_tessellationInfo = {};
    VkPipelineViewportStateCreateInfo m_viewportInfo = {};
    VkPipelineRasterizationStateCreateInfo m_rasterizationInfo = {};
    VkPipelineMultisampleStateCreateInfo m_multisampleInfo = {};
    VkPipelineDepthStencilStateCreateInfo m_depthStencilInfo = {};
    VkPipelineColorBlendStateCreateInfo m_colorBlendInfo = {};
    VkPipelineDynamicStateCreateInfo m_dynamicInfo = {};
    VkPipelineRenderingCreateInfoKHR m_renderingInfo = {};

public:
    GraphicsPipelineCreateInfo(Device* device);
    ~GraphicsPipelineCreateInfo();

    const VkGraphicsPipelineCreateInfo& Setup();

    struct ShaderStage
    {
        VkShaderStageFlagBits m_stage;
        rad::Ref<ShaderModule> shader;
        rad::Ref<SpecializationInfo> specialization;
    };
    std::vector<ShaderStage> m_shaderStages;

    struct VertexInputInfo
    {
        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;
    } m_vertexInput;
    // the vertex attributes are compacted in the order of formats.
    void AddVertexBindingPacked(
        uint32_t binding, rad::Span<VertexInputAttrib> attribs,
        VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX);

    struct InputAssemblyInfo
    {
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkBool32 primitiveRestartEnable = VK_FALSE;
    } m_inputAssembly;

    struct TessellationInfo
    {
        uint32_t patchControlPoints = 0;
    } m_tessellation;

    // viewports and scissors should be set dynamically.
    uint32_t m_viewportCount = 1;
    uint32_t m_scissorCount = 1;

    struct RasterizationInfo
    {
        VkBool32                depthClampEnable = VK_FALSE;
        VkBool32                rasterizerDiscardEnable = VK_FALSE;
        VkPolygonMode           polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags         cullMode = VK_CULL_MODE_NONE;
        VkFrontFace             frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        VkBool32                depthBiasEnable = VK_FALSE;
        float                   depthBiasConstantFactor = 0;
        float                   depthBiasClamp = 0;
        float                   depthBiasSlopeFactor = 0;
        float                   lineWidth = 1.0f;
    } m_rasterization;

    struct MultisampleInfo
    {
        VkSampleCountFlagBits   rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        VkBool32                sampleShadingEnable = VK_FALSE;
        float                   minSampleShading = 0.0f;    // must be in the range [0,1]
        VkSampleMask            sampleMask = 0xFFFFFFFF;
        VkBool32                alphaToCoverageEnable = VK_FALSE;
        VkBool32                alphaToOneEnable = VK_FALSE;
    } m_multisample;

    struct DepthStencilInfo
    {
        VkBool32                depthTestEnable = VK_FALSE;
        VkBool32                depthWriteEnable = VK_FALSE;
        VkCompareOp             depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        VkBool32                depthBoundsTestEnable = VK_FALSE;
        VkBool32                stencilTestEnable = VK_FALSE;
        VkStencilOpState        front;
        VkStencilOpState        back;
        float                   minDepthBounds = 0.0f;
        float                   maxDepthBounds = 1.0f;
    } m_depthStencil;

    struct ColorBlendInfo
    {
        VkBool32                logicOpEnable = VK_FALSE;
        VkLogicOp               logicOp;
        std::vector<VkPipelineColorBlendAttachmentState> attachments;
        float                   blendConstants[4];
    } m_colorBlend;
    void SetColorBlendDisabled(uint32_t attachCount);

    std::vector<VkDynamicState> m_dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    rad::Ref<PipelineLayout>    m_layout;
    rad::Ref<RenderPass>        m_renderPass;
    uint32_t                    m_subpass = 0;
    rad::Ref<Pipeline>          m_basePipeline;
    int32_t                     m_basePipelineIndex = 0;

    std::vector<VkFormat> m_colorFormats;
    void SetRenderingInfo(
        rad::Span<VkFormat> colorFormats,
        VkFormat depthStencilFormat = VK_FORMAT_UNDEFINED);

}; // struct GraphicsPipelineCreateInfo

struct ComputePipelineCreateInfo : public rad::RefCounted<ComputePipelineCreateInfo>
{
private:
    Device* m_device;
    VkComputePipelineCreateInfo m_pipelineInfo = {};
    VkSpecializationInfo m_specializationInfo = {};
public:
    ComputePipelineCreateInfo(Device* device);
    ~ComputePipelineCreateInfo();

    const VkComputePipelineCreateInfo& Setup();

    rad::Ref<ShaderModule>          m_shaderModule;
    rad::Ref<SpecializationInfo>    m_shaderSpecialization;
    rad::Ref<PipelineLayout>        m_layout;
    rad::Ref<Pipeline>              m_basePipeline;
    int32_t                         m_basePipelineIndex = 0;

}; // struct ComputePipelineCreateInfo

} // namespace vkpp
