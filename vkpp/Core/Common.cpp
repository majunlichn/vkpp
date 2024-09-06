#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include <vkpp/Core/Common.h>

namespace vkpp
{

spdlog::logger* GetLogger()
{
    static std::shared_ptr<spdlog::logger> logger = rad::CreateLogger("vkpp");
    return logger.get();
}

void ReportError(VkResult result, const char* function, const char* file, uint32_t line)
{
    if (result < 0)
    {
        RAD_LOG(GetLogger(), err, "{} failed with VkResult={}({}, line {}).",
            function, string_VkResult(result), file, line);
        throw VulkanError(result);
    }
}

bool IsVersionMatchOrGreater(uint32_t version, uint32_t major, uint32_t minor, uint32_t patch)
{
    return (VK_VERSION_MAJOR(version) >= major) &&
        (VK_VERSION_MINOR(version) >= minor) &&
        (VK_VERSION_PATCH(version) >= patch);
}

std::vector<VkLayerProperties> EnumerateInstanceLayers()
{
    std::vector<VkLayerProperties> layers;
    uint32_t count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&count, nullptr));
    if (count > 0)
    {
        layers.resize(count);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&count, layers.data()));
    }
    return layers;
}

std::vector<VkExtensionProperties> EnumerateInstanceExtensions(const char* layerName)
{
    std::vector<VkExtensionProperties> extensions;
    uint32_t count = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(layerName, &count, nullptr));
    if (count > 0)
    {
        extensions.resize(count);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(layerName, &count, extensions.data()));
    }
    return extensions;
}

bool HasLayer(rad::Span<VkLayerProperties> layer, std::string_view name)
{
    for (const VkLayerProperties& layer : layer)
    {
        if (rad::StrEqual(layer.layerName, name))
        {
            return true;
        }
    }
    return false;
}

bool HasExtension(rad::Span<VkExtensionProperties> extensions, std::string_view name)
{
    for (const VkExtensionProperties& extension : extensions)
    {
        if (rad::StrEqual(extension.extensionName, name))
        {
            return true;
        }
    }
    return false;
}

} // namespace vkpp
