#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class Framebuffer : public rad::RefCounted<Framebuffer>
{
public:
    Framebuffer(
        rad::Ref<Device> device,
        const VkFramebufferCreateInfo& createInfo);
    ~Framebuffer();
    VKPP_DISABLE_COPY_AND_MOVE(Framebuffer);

    VkFramebuffer GetHandle() const { return m_handle; }

    uint32_t GetAttachmentCount() const { return m_attachmentCount; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    uint32_t GetLayers() const { return m_layers; }

private:
    rad::Ref<Device>        m_device;
    VkFramebuffer           m_handle = VK_NULL_HANDLE;
    uint32_t                m_attachmentCount = 0;
    uint32_t                m_width = 0;
    uint32_t                m_height = 0;
    uint32_t                m_layers = 0;

}; // class Framebuffer

} // namespace vkpp
