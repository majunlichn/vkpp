#include <vkpp/Core/RenderPass.h>
#include <vkpp/Core/Device.h>

namespace vkpp
{

RenderPass::RenderPass(
    rad::Ref<Device> device,
    const VkRenderPassCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateRenderPass(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

RenderPass::~RenderPass()
{
    m_device->GetFunctionTable()->
        vkDestroyRenderPass(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}

} // namespace vkpp
