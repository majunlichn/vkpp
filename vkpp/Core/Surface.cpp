#include <vkpp/Core/Surface.h>
#include <vkpp/Core/Instance.h>

namespace vkpp
{

Surface::Surface(rad::Ref<Instance> instance, VkSurfaceKHR handle) :
    m_instance(std::move(instance)),
    m_handle(handle)
{
}

Surface::~Surface()
{
    vkDestroySurfaceKHR(m_instance->GetHandle(), m_handle, nullptr);
}

} // namespace vkpp
