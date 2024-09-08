#include <vkpp/Core/Semaphore.h>
#include <vkpp/Core/Device.h>

namespace vkpp
{

Semaphore::Semaphore(
    rad::Ref<Device> device, const VkSemaphoreCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateSemaphore(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

Semaphore::~Semaphore()
{
    if (m_handle != VK_NULL_HANDLE)
    {
        m_device->GetFunctionTable()->
            vkDestroySemaphore(m_device->GetHandle(), m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
    }
}

} // namespace vkpp
