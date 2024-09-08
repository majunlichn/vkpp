#pragma once

#include <vkpp/Core/Common.h>
#include <set>

namespace vkpp
{

class Instance : public rad::RefCounted<Instance>
{
public:
    Instance();
    ~Instance();
    VKPP_DISABLE_COPY_AND_MOVE(Instance);

    bool Init(std::string_view appName, uint32_t appVersion);

    VkInstance GetHandle() { return m_handle; }
    uint32_t GetVersion() const { return m_version; }
    bool IsVersionMatchOrGreater(uint32_t major, uint32_t minor, uint32_t patch);
    bool IsExtensionSupported(std::string_view name) const;

    std::vector<rad::Ref<PhysicalDevice>> EnumeratePhysicalDevices();

private:
    VkInstance m_handle = VK_NULL_HANDLE;
    uint32_t m_version = 0;
    std::set<std::string, rad::StringLess> m_enabledExtensions;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

}; // class Instance

} // namespace vkpp
