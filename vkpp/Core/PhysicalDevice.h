#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class PhysicalDevice : public rad::RefCounted<PhysicalDevice>
{
public:
    PhysicalDevice(rad::Ref<Instance> instance, VkPhysicalDevice handle);
    ~PhysicalDevice();
    VKPP_DISABLE_COPY_AND_MOVE(PhysicalDevice);

    VkPhysicalDevice GetHandle() { return m_handle; }
    bool IsVersionMatchOrGreater(uint32_t major, uint32_t minor, uint32_t patch);
    uint32_t GetVendorID() const { return m_properties.vendorID; }
    uint32_t GetDeviceID() const { return m_properties.deviceID; }
    VkPhysicalDeviceType GetDeviceType() const { return m_properties.deviceType; }
    bool IsIntegrated() const;
    bool IsDiscrete() const;
    // the device is typically a virtual node in a virtualization environment.
    bool IsVirtual() const;
    bool IsCPU() const;
    const char* GetDeviceName() const { return m_properties.deviceName; }

    bool IsExtensionSupported(std::string_view name) const;

    VkFormatProperties GetFormatProperties(VkFormat format) const;
    bool IsFormatSupported(VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(VkSurfaceKHR surface);
    std::vector<VkPresentModeKHR> GetSurfacePresentModes(VkSurfaceKHR surface);
    std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(VkSurfaceKHR surface);

    rad::Ref<Device> CreateDevice(const std::set<std::string>& extensionNames);

private:
    rad::Ref<Instance> m_instance;
    VkPhysicalDevice m_handle = VK_NULL_HANDLE;

public:
    VkPhysicalDeviceProperties m_properties;
    VkPhysicalDeviceProperties2 m_properties2;
    VkPhysicalDeviceVulkan11Properties m_vk11Properties;
    VkPhysicalDeviceVulkan12Properties m_vk12Properties;
    VkPhysicalDeviceVulkan13Properties m_vk13Properties;
    std::vector<VkQueueFamilyProperties> m_queueFamilies;
    VkPhysicalDeviceMemoryProperties m_memoryProperties;
    std::vector<VkExtensionProperties> m_extensions;

    VkPhysicalDeviceFeatures m_features;
    VkPhysicalDeviceFeatures2 m_features2;
    VkPhysicalDeviceVulkan11Features m_vk11Features;
    VkPhysicalDeviceVulkan12Features m_vk12Features;
    VkPhysicalDeviceVulkan13Features m_vk13Features;
    VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR m_barycentricFeatures = {};

}; // class PhysicalDevice

} // namespace vkpp
