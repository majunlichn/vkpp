#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

struct SubmitWaitInfo
{
    Semaphore* semaphore;
    VkPipelineStageFlags dstStageMask;
};

class Queue : public rad::RefCounted<Queue>
{
public:
    Queue(rad::Ref<Device> device, QueueFamily queueFamily);
    ~Queue();
    VKPP_DISABLE_COPY_AND_MOVE(Queue);

    VkQueue GetHandle() const { return m_handle; }

    QueueFamily GetQueueFamily() const;
    uint32_t GetQueueFamilyIndex() const;
    const VkQueueFamilyProperties& GetQueueFamilyProperties() const;
    bool SupportGraphics() const;
    bool SupportCompute() const;

    void Submit(
        rad::Span<CommandBuffer*>   commandBuffers,
        rad::Span<SubmitWaitInfo>   waits = {},
        rad::Span<Semaphore*>       signalSemaphores = {},
        Fence* fence = nullptr
    );

    // Create a fence implicitly; wait the GPU to complete the commands and notify the host.
    void SubmitAndWait(
        rad::Span<CommandBuffer*>   commandBuffers,
        rad::Span<SubmitWaitInfo>   waits = {},
        rad::Span<Semaphore*>       signalSemaphores = {}
    );

    VkResult WaitIdle();

    VkResult Present(
        rad::Span<Semaphore*>       waitSemaphores,
        rad::Span<Swapchain*>       swapchains,
        rad::Span<uint32_t>         imageIndices,
        VkResult* pResults);
    VkResult Present(
        rad::Span<Semaphore*>       waitSemaphores,
        rad::Span<Swapchain*>       swapchains);

private:
    rad::Ref<Device>        m_device;
    QueueFamily             m_queueFamily = QueueFamilyUniversal;
    VkQueue                 m_handle = VK_NULL_HANDLE;

}; // class Queue

} // namespace vkpp
