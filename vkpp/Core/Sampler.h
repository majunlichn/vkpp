#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class Sampler : public rad::RefCounted<Sampler>
{
public:
    Sampler(rad::Ref<Device> device, const VkSamplerCreateInfo& createInfo);
    ~Sampler();
    VKPP_DISABLE_COPY_AND_MOVE(Sampler);

    VkSampler GetHandle() const { return m_handle; }

private:
    rad::Ref<Device> m_device;
    VkSampler m_handle = VK_NULL_HANDLE;

}; // class Sampler

} // namespace vkpp
