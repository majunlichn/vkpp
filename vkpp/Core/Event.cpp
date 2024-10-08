#include <vkpp/Core/Event.h>
#include <vkpp/Core/Device.h>

namespace vkpp
{

Event::Event(rad::Ref<Device> device, const VkEventCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateEvent(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

Event::~Event()
{
    if (m_handle != VK_NULL_HANDLE)
    {
        m_device->GetFunctionTable()->
            vkDestroyEvent(m_device->GetHandle(), m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
    }
}

} // namespace vkpp
