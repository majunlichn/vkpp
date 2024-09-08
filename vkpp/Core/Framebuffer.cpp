#include <vkpp/Core/Framebuffer.h>
#include <vkpp/Core/Device.h>

namespace vkpp
{

Framebuffer::Framebuffer(
    rad::Ref<Device> device, const VkFramebufferCreateInfo& createInfo) :
    m_device(device)
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateFramebuffer(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
    m_attachmentCount = createInfo.attachmentCount;
    m_width = createInfo.width;
    m_height = createInfo.height;
    m_layers = createInfo.layers;
}

Framebuffer::~Framebuffer()
{
    m_device->GetFunctionTable()->
        vkDestroyFramebuffer(m_device->GetHandle(), m_handle, nullptr);
}

} // namespace vkpp
