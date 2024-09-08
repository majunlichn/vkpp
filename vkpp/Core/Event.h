#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class Event : public rad::RefCounted<Event>
{
public:
    Event(rad::Ref<Device> device, const VkEventCreateInfo& createInfo);
    ~Event();
    VKPP_DISABLE_COPY_AND_MOVE(Event);

    VkEvent GetHandle() const { return m_handle; }

private:
    rad::Ref<Device> m_device;
    VkEvent m_handle = VK_NULL_HANDLE;

}; // class Event

} // namespace vkpp
