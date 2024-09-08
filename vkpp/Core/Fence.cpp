#include <vkpp/Core/Fence.h>
#include <vkpp/Core/Device.h>

namespace vkpp
{

Fence::Fence(rad::Ref<Device> device, const VkFenceCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateFence(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

Fence::~Fence()
{
    m_device->GetFunctionTable()->
        vkDestroyFence(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}

void Fence::Wait(uint64_t timeout)
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkWaitForFences(m_device->GetHandle(), 1, &m_handle, true, timeout));
}

void Fence::Reset()
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkResetFences(m_device->GetHandle(), 1, &m_handle));
}

} // namespace vkpp
