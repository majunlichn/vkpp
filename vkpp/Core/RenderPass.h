#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class RenderPass : public rad::RefCounted<RenderPass>
{
public:
    RenderPass(rad::Ref<Device> device,
        const VkRenderPassCreateInfo& createInfo);
    ~RenderPass();
    VKPP_DISABLE_COPY_AND_MOVE(RenderPass);

    VkRenderPass GetHandle() const { return m_handle; }

private:
    rad::Ref<Device> m_device;
    VkRenderPass m_handle = VK_NULL_HANDLE;

}; // class RenderPass

} // namespace vkpp
