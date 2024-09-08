#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class Semaphore : public rad::RefCounted<Semaphore>
{
public:
    Semaphore(rad::Ref<Device> device, const VkSemaphoreCreateInfo& createInfo);
    ~Semaphore();
    VKPP_DISABLE_COPY_AND_MOVE(Semaphore);

    VkSemaphore GetHandle() const { return m_handle; }

private:
    rad::Ref<Device> m_device;
    VkSemaphore m_handle = VK_NULL_HANDLE;

}; // class Semaphore

} // namespace vkpp
