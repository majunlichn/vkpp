#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class Surface : public rad::RefCounted<Surface>
{
public:
    Surface(rad::Ref<Instance> instance, VkSurfaceKHR handle);
    ~Surface();
    VKPP_DISABLE_COPY_AND_MOVE(Surface);

    VkSurfaceKHR GetHandle() const { return m_handle; }

private:
    rad::Ref<Instance> m_instance;
    VkSurfaceKHR m_handle = VK_NULL_HANDLE;

}; // class Surface

} // namespace vkpp
