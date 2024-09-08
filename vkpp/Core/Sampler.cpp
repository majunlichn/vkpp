#include <vkpp/Core/Sampler.h>
#include <vkpp/Core/Device.h>

namespace vkpp
{

Sampler::Sampler(rad::Ref<Device> device, const VkSamplerCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateSampler(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

Sampler::~Sampler()
{
    m_device->GetFunctionTable()->
        vkDestroySampler(m_device->GetHandle(), m_handle, nullptr);
}

} // namespace vkpp
