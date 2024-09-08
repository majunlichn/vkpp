#include <vkpp/Core/Instance.h>
#include <vkpp/Core/PhysicalDevice.h>
#include <vkpp/Core/Device.h>
#include <rad/System/Application.h>

int main(int argc, char* argv[])
{
    rad::Application app;
    if (!app.Init(argc, argv))
    {
        RAD_LOG_DEFAULT(err, "rad::Application::Init failed!");
        return -1;
    }

    rad::Ref<vkpp::Instance> instance = RAD_NEW vkpp::Instance();
    if (instance->Init("VulkanViewer", VK_MAKE_VERSION(0, 0, 0)))
    {
        VKPP_LOG(info, "Vulkan instance initialized.");
    }

    auto gpus = instance->EnumeratePhysicalDevices();
    for (size_t i = 0; i < gpus.size(); ++i)
    {
        VKPP_LOG(info, "Vulkan device#{}: {} (0x{:04X})", i,
            gpus[i]->GetDeviceName(), gpus[i]->GetDeviceID());
    }

    rad::Ref<vkpp::PhysicalDevice> gpuSelected;
    for (auto& gpu : gpus)
    {
        const VkPhysicalDeviceProperties& properties = gpu->m_properties;
        // Prefer the first discrete GPU.
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            gpuSelected = gpu;
            break;
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

    if (!instance->IsExtensionSupported("VK_KHR_get_physical_device_properties2") ||
        !instance->IsExtensionSupported("VK_KHR_surface") ||
        !instance->IsExtensionSupported("VK_KHR_get_surface_capabilities2") ||
        !instance->IsExtensionSupported("VK_KHR_swapchain"))
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

    rad::Ref<vkpp::Device> device = gpuSelected->CreateDevice(extensionNames);

    return 0;
}
