#include "Context.h"

namespace vkpp
{

Context::Context()
{
}

Context::~Context()
{
}

bool Context::Init(rad::Ref<Instance> instance, rad::Ref<PhysicalDevice> gpuSelected)
{
    m_instance = std::move(instance);

    if (!gpuSelected)
    {
        auto gpus = m_instance->EnumeratePhysicalDevices();
        for (size_t i = 0; i < gpus.size(); ++i)
        {
            VKPP_LOG(info, "Vulkan device#{}: {} (0x{:04X})", i,
                gpus[i]->GetDeviceName(), gpus[i]->GetDeviceID());
        }

        if (gpus.empty())
        {
            VKPP_LOG(err, "No Vulkan device available!");
            return false;
        }

        // Prefer the first discrete GPU.
        for (auto& gpu : gpus)
        {
            const VkPhysicalDeviceProperties& properties = gpu->m_properties;
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                gpuSelected = gpu;
                break;
            }
        }

        if (!gpuSelected)
        {
            gpuSelected = gpus[0];
        }
    }

    uint32_t apiVersion = gpuSelected->m_properties.apiVersion;
    VKPP_LOG(info, "Device selected: {} (version={}.{}.{})",
        gpuSelected->GetDeviceName(),
        VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion));

    std::set<std::string> extensionNames;
    for (const VkExtensionProperties& extension : gpuSelected->m_extensions)
    {
        if (std::string_view(extension.extensionName).starts_with("VK_KHR") ||
            std::string_view(extension.extensionName).starts_with("VK_EXT"))
        {
            extensionNames.insert(extension.extensionName);
        }
    }

    if (!m_instance->IsExtensionSupported("VK_KHR_get_physical_device_properties2") ||
        !m_instance->IsExtensionSupported("VK_KHR_surface") ||
        !m_instance->IsExtensionSupported("VK_KHR_get_surface_capabilities2") ||
        !m_instance->IsExtensionSupported("VK_KHR_swapchain"))
    {
        extensionNames.erase("VK_EXT_full_screen_exclusive");
    }

    if (extensionNames.find("VK_EXT_surface_maintenance1") == extensionNames.end())
    {
        extensionNames.erase("VK_EXT_swapchain_maintenance1");
    }

    if (extensionNames.find("VK_KHR_buffer_device_address") != extensionNames.end())
    {
        extensionNames.erase("VK_EXT_buffer_device_address");
    }

    m_device = gpuSelected->CreateDevice(extensionNames);

    for (uint32_t i = 0; i < QueueFamilyCount; ++i)
    {
        QueueFamily queueFamily = QueueFamily(i);
        if (m_device->IsQueueFamilySupported(queueFamily))
        {
            m_queues[i] = m_device->CreateQueue(QueueFamily(queueFamily));
        }
    }

    return true;
}

} // namespace vkpp
