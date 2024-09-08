#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class Device : public rad::RefCounted<Device>
{
public:
    Device(
        rad::Ref<Instance> instance,
        rad::Ref<PhysicalDevice> physicalDevice,
        const std::set<std::string>& extensionNames);
    ~Device();
    VKPP_DISABLE_COPY_AND_MOVE(Device);

    VkDevice GetHandle() const { return m_handle; }
    const VolkDeviceTable* GetFunctionTable() const { return &m_functionTable; }
    VmaAllocator GetAllocator() const { return m_allocator; }

    PhysicalDevice* GetPhysicalDevice() const { return m_physicalDevice.get(); }
    const char* GetDeviceName() const;
    const VkPhysicalDeviceLimits& GetLimits() const;

    bool IsExtensionSupported(std::string_view extension);

    bool IsQueueFamilySupported(QueueFamily queueFamily) const;
    uint32_t GetQueueFamilyIndex(QueueFamily queueFamily) const;
    rad::Ref<Queue> CreateQueue(QueueFamily queueFamily);
    bool IsSurfaceSupported(QueueFamily queueFamily, Surface* surface);

    rad::Ref<CommandPool> CreateCommandPool(
        QueueFamily queueFamily = QueueFamilyUniversal,
        VkCommandPoolCreateFlags flags = 0);

    // Synchronization and Cache Control
    rad::Ref<Fence> CreateFence(VkFenceCreateFlags flags = 0);
    rad::Ref<Semaphore> CreateSemaphore(VkSemaphoreCreateFlags flags = 0);
    rad::Ref<Semaphore> CreateSemaphoreSignaled();
    rad::Ref<Event> CreateEvent();
    void WaitIdle();

    // RenderPass
    rad::Ref<RenderPass> CreateRenderPass(const VkRenderPassCreateInfo& createInfo);
    rad::Ref<Framebuffer> CreateFramebuffer(
        RenderPass* renderPass,
        rad::Span<ImageView*> attachments,
        uint32_t width,
        uint32_t height,
        uint32_t layers);

    // Piplines
    rad::Ref<ShaderModule> CreateShaderModule(rad::Span<uint32_t> code);
    rad::Ref<GraphicsPipeline> CreateGraphicsPipeline(
        const VkGraphicsPipelineCreateInfo& createInfo);
    rad::Ref<ComputePipeline> CreateComputePipeline(
        const VkComputePipelineCreateInfo& createInfo);

    // Resource
    rad::Ref<Buffer> CreateBuffer(
        const VkBufferCreateInfo& createInfo,
        const VmaAllocationCreateInfo& allocInfo);
    rad::Ref<Buffer> CreateBuffer(
        const VkBufferCreateInfo& createInfo,
        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);
    rad::Ref<Buffer> CreateBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);
    rad::Ref<Buffer> CreateUniformBuffer(VkDeviceSize size, bool isPersistentMapped = false);
    rad::Ref<Buffer> CreateStagingBuffer(VkDeviceSize size);
    rad::Ref<Buffer> CreateStorageBuffer(VkDeviceSize size);
    rad::Ref<Buffer> CreateVertexBuffer(VkDeviceSize size);
    rad::Ref<Buffer> CreateIndexBuffer(VkDeviceSize size);

    rad::Ref<Image> CreateImage(
        const VkImageCreateInfo& createInfo,
        const VmaAllocationCreateInfo& allocInfo);
    rad::Ref<Image> CreateImage(
        const VkImageCreateInfo& createInfo,
        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);
    rad::Ref<Image> CreateImageFromHandle(
        const VkImageCreateInfo& createInfo,
        VkImage imageHandle);
    rad::Ref<Image> CreateImage2DRenderTarget(
        VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags otherUsages = 0);
    rad::Ref<Image> CreateImage2DDepthStencil(
        VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags otherUsages = 0);
    rad::Ref<Image> CreateImage2DTexture(
        VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels);

    // Samplers
    rad::Ref<Sampler> CreatSampler(const VkSamplerCreateInfo& createInfo);
    rad::Ref<Sampler> CreatSamplerNearest(
        VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
    rad::Ref<Sampler> CreatSamplerLinear(
        VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        float maxAnisotropy = 0.0f);

    // Resource Binding
    rad::Ref<DescriptorSetLayout> CreateDescriptorSetLayout(
        rad::Span<VkDescriptorSetLayoutBinding> layoutBindings);
    rad::Ref<PipelineLayout> CreatePipelineLayout(
        rad::Span<DescriptorSetLayout*> descSetLayouts,
        rad::Span<VkPushConstantRange> pushConstantRanges = {});
    rad::Ref<DescriptorPool> CreateDescriptorPool(
        const VkDescriptorPoolCreateInfo& createInfo);
    rad::Ref<DescriptorPool> CreateDescriptorPool(
        uint32_t maxSets, rad::Span<VkDescriptorPoolSize> poolSizes);

private:
    rad::Ref<Instance> m_instance;
    rad::Ref<PhysicalDevice> m_physicalDevice;
    VkDevice m_handle = VK_NULL_HANDLE;
    // Map QueueFamily to the real queue family index.
    uint32_t m_queueFamilyIndexTable[QueueFamilyCount];
    std::set<std::string, rad::StringLess> m_enabledExtensionNames;
    VolkDeviceTable m_functionTable = {};
    VmaAllocator m_allocator = nullptr;

}; // class Device

} // namespace vkpp
