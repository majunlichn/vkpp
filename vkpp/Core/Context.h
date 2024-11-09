#pragma once

#include <vkpp/Core/Instance.h>
#include <vkpp/Core/PhysicalDevice.h>
#include <vkpp/Core/Device.h>
#include <vkpp/Core/Queue.h>
#include <vkpp/Core/Command.h>
#include <vkpp/Core/Fence.h>
#include <vkpp/Core/Semaphore.h>
#include <vkpp/Core/Event.h>
#include <vkpp/Core/RenderPass.h>
#include <vkpp/Core/Framebuffer.h>
#include <vkpp/Core/Pipeline.h>
#include <vkpp/Core/Buffer.h>
#include <vkpp/Core/Image.h>
#include <vkpp/Core/Sampler.h>
#include <vkpp/Core/Descriptor.h>
#include <vkpp/Core/Surface.h>
#include <vkpp/Core/Swapchain.h>

#include <mutex>

namespace vkpp
{

class Context : public rad::RefCounted<Context>
{
public:
    Context();
    virtual ~Context();
    VKPP_DISABLE_COPY_AND_MOVE(Context);

    bool Init(rad::Ref<Instance> instance, rad::Ref<PhysicalDevice> gpu);
    Instance* GetInstance() { return m_instance.get(); }
    Device* GetDevice() { return m_device.get(); }
    Queue* GetQueue(QueueFamily family = QueueFamilyUniversal)
    {
        return m_queues[family].get();
    }
    rad::Ref<CommandBuffer> AllocateTransientCommandBuffer(
        QueueFamily queueFamily = QueueFamilyUniversal,
        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    void WaitIdle();

    void ReadBuffer(Buffer* buffer, void* dest, VkDeviceSize offset, VkDeviceSize size);
    void ReadBuffer(Buffer* buffer, void* dest);
    void WriteBuffer(Buffer* buffer, const void* data, VkDeviceSize offset, VkDeviceSize size);
    void WriteBuffer(Buffer* buffer, const void* data);

    void CopyBufferToImage(Buffer* buffer, Image* image, rad::Span<VkBufferImageCopy> copyInfos);
    void CopyBufferToImage2D(Buffer* buffer, VkDeviceSize bufferOffset,
        Image* image, uint32_t baseMipLevel = 0, uint32_t levelCount = 1,
        uint32_t baseArrayLayer = 0, uint32_t layerCount = 1);

    rad::Ref<Instance> m_instance;
    rad::Ref<Device> m_device;
    rad::Ref<Queue> m_queues[QueueFamilyCount];
    // Host access to command pools must be externally synchronized.
    std::mutex m_cmdPoolMutex;
    rad::Ref<CommandPool> m_cmdPools[QueueFamilyCount];

    VkExtent2D m_resolution = {};
    uint32_t m_swapchainImageCount = 3;
    VkFormat m_colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat m_depthStencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

}; // class Context

} // namespace vkpp
