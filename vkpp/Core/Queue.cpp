#include <vkpp/Core/Queue.h>
#include <vkpp/Core/PhysicalDevice.h>
#include <vkpp/Core/Device.h>
#include <vkpp/Core/Command.h>
#include <vkpp/Core/Fence.h>
#include <vkpp/Core/Semaphore.h>
#include <vkpp/Core/Event.h>
#include <vkpp/Core/Swapchain.h>

#include <rad/Container/SmallVector.h>

namespace vkpp
{

Queue::Queue(rad::Ref<Device> device, QueueFamily queueFamily) :
    m_device(std::move(device))
{
    uint32_t queueFamilyIndex = m_device->GetQueueFamilyIndex(queueFamily);
    m_device->GetFunctionTable()->
        vkGetDeviceQueue(m_device->GetHandle(),
            queueFamilyIndex, 0, &m_handle);
}

Queue::~Queue()
{
}

QueueFamily Queue::GetQueueFamily() const
{
    return m_queueFamily;
}

uint32_t Queue::GetQueueFamilyIndex() const
{
    return m_device->GetQueueFamilyIndex(m_queueFamily);
}

const VkQueueFamilyProperties& Queue::GetQueueFamilyProperties() const
{
    uint32_t queueFamilyIndex = GetQueueFamilyIndex();
    return m_device->GetPhysicalDevice()->m_queueFamilies[queueFamilyIndex];
}

bool Queue::SupportGraphics() const
{
    return rad::HasBits<uint32_t>(GetQueueFamilyProperties().queueFlags, VK_QUEUE_GRAPHICS_BIT);
}

bool Queue::SupportCompute() const
{
    return rad::HasBits<uint32_t>(GetQueueFamilyProperties().queueFlags, VK_QUEUE_COMPUTE_BIT);
}

void Queue::Submit(
    rad::Span<CommandBuffer*>   commandBuffers,
    rad::Span<SubmitWaitInfo>   waits,
    rad::Span<Semaphore*>       signalSemaphores,
    Fence* fence)
{
    rad::SmallVector<VkCommandBuffer, 8> commandBufferHandles(commandBuffers.size());
    for (int i = 0; i < commandBuffers.size(); i++)
    {
        commandBufferHandles[i] = commandBuffers[i]->GetHandle();
    }

    rad::SmallVector<VkSemaphore, 8> waitSemaphoreHandles(waits.size());
    rad::SmallVector<VkPipelineStageFlags, 8> waitDstStageMasks(waits.size());
    for (int i = 0; i < waits.size(); i++)
    {
        waitSemaphoreHandles[i] = waits[i].semaphore->GetHandle();
        waitDstStageMasks[i] = waits[i].dstStageMask;
    }

    rad::SmallVector<VkSemaphore, 8> signalSemaphoresHandles(signalSemaphores.size());
    for (int i = 0; i < signalSemaphores.size(); i++)
    {
        signalSemaphoresHandles[i] = signalSemaphores[i]->GetHandle();
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphoreHandles.size());
    submitInfo.pWaitSemaphores = waitSemaphoreHandles.data();
    submitInfo.pWaitDstStageMask = waitDstStageMasks.data();
    submitInfo.commandBufferCount = static_cast<uint32_t>(commandBufferHandles.size());
    submitInfo.pCommandBuffers = commandBufferHandles.data();
    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphoresHandles.size());
    submitInfo.pSignalSemaphores = signalSemaphoresHandles.data();

    VK_CHECK(m_device->GetFunctionTable()->
        vkQueueSubmit(m_handle, 1, &submitInfo, fence ? fence->GetHandle() : VK_NULL_HANDLE));
}

void Queue::SubmitAndWait(
    rad::Span<CommandBuffer*>   commandBuffers,
    rad::Span<SubmitWaitInfo>   waits,
    rad::Span<Semaphore*>       signalSemaphores)
{
    rad::Ref<Fence> fence = m_device->CreateFence();
    Submit(commandBuffers, waits, signalSemaphores, fence.get());
    fence->Wait();
}

VkResult Queue::WaitIdle()
{
    return vkQueueWaitIdle(m_handle);
}

VkResult Queue::Present(
    rad::Span<Semaphore*> waitSemaphores,
    rad::Span<Swapchain*> swapchains,
    rad::Span<uint32_t> imageIndices,
    VkResult* pResults)
{
    assert(swapchains.size() == imageIndices.size());

    rad::SmallVector<VkSemaphore, 8> waitSemaphoresHandles(waitSemaphores.size());
    for (int i = 0; i < waitSemaphores.size(); i++)
    {
        waitSemaphoresHandles[i] = waitSemaphores[i]->GetHandle();
    }

    rad::SmallVector<VkSwapchainKHR, 8> swapchainsHandles(swapchains.size());
    for (int i = 0; i < swapchains.size(); i++)
    {
        swapchainsHandles[i] = swapchains[i]->GetHandle();
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount =
        static_cast<uint32_t>(waitSemaphoresHandles.size());
    presentInfo.pWaitSemaphores = waitSemaphoresHandles.data();
    presentInfo.swapchainCount =
        static_cast<uint32_t>(swapchainsHandles.size());
    presentInfo.pSwapchains = swapchainsHandles.data();
    presentInfo.pImageIndices = imageIndices.data();
    presentInfo.pResults = pResults;

    return m_device->GetFunctionTable()->
        vkQueuePresentKHR(m_handle, &presentInfo);
}

VkResult Queue::Present(
    rad::Span<Semaphore*> waitSemaphores,
    rad::Span<Swapchain*> swapchains)
{
    rad::SmallVector<uint32_t, 8> imageIndices;
    imageIndices.resize(swapchains.size());
    for (int i = 0; i < swapchains.size(); i++)
    {
        imageIndices[i] = swapchains[i]->GetCurrentImageIndex();
    }
    return Present(waitSemaphores, swapchains, imageIndices, nullptr);
}

} // namespace vkpp
