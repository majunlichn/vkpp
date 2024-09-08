#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class Fence : public rad::RefCounted<Fence>
{
public:
    Fence(rad::Ref<Device> device, const VkFenceCreateInfo& createInfo);
    ~Fence();
    VKPP_DISABLE_COPY_AND_MOVE(Fence);

    VkFence GetHandle() const { return m_handle; }

    // @param timeout: in nanoseconds, will be adjusted to the closest value allowed by implementation.
    void Wait(uint64_t timeout = UINT64_MAX);
    void Reset();

private:
    rad::Ref<Device> m_device;
    VkFence m_handle = VK_NULL_HANDLE;

}; // class Fence

} // namespace vkpp
