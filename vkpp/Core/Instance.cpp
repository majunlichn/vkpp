#include <vkpp/Core/Instance.h>
#include <vkpp/Core/PhysicalDevice.h>
#if defined(RAD_OS_WINDOWS)
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

namespace vkpp
{

Instance::Instance()
{
}

Instance::~Instance()
{
    volkFinalize();
}

std::string getenv(std::string_view name)
{
#if defined(RAD_OS_WINDOWS)
    char* buffer = nullptr;
    size_t size = 0;
    errno_t err = _dupenv_s(&buffer, &size, name.data());
    if ((err == 0) && (buffer != nullptr))
    {
        std::string value(buffer);
        free(buffer);
        return value;
    }
    else
    {
        return std::string();
    }
#else
    const char* value = std::getenv(name.data());
    if (value)
    {
        return std::string(value);
    }
    else
    {
        return std::string();
    }
#endif
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    switch (severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        RAD_LOG(GetLogger(), debug, "[{}] {}",
            pCallbackData->pMessageIdName, pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        RAD_LOG(GetLogger(), info, "[{}] {}",
            pCallbackData->pMessageIdName, pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        RAD_LOG(GetLogger(), warn, "[{}] {}",
            pCallbackData->pMessageIdName, pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        RAD_LOG(GetLogger(), err, "[{}] {}",
            pCallbackData->pMessageIdName, pCallbackData->pMessage);
#if defined(_DEBUG)
#if defined(RAD_COMPILER_MSVC)
        __debugbreak();
#endif
#endif
        break;
    }

    return VK_FALSE;
}

bool Instance::Init(std::string_view appName, uint32_t appVersion)
{
    VK_CHECK(volkInitialize());

    if (vkEnumerateInstanceVersion)
    {
        vkEnumerateInstanceVersion(&m_version);
        RAD_LOG(GetLogger(), info, "Vulkan instance version: {}.{}.{}.{}",
            VK_API_VERSION_VARIANT(m_version),
            VK_VERSION_MAJOR(m_version),
            VK_VERSION_MINOR(m_version),
            VK_VERSION_PATCH(m_version)
        );
    }

    std::vector<const char*> enabledLayers;

    std::vector<VkLayerProperties> layers = EnumerateInstanceLayers();

#ifdef _DEBUG
    bool enableValidation = true;
#else
    bool enableValidation = false;
#endif
    std::string envEnableValidation = getenv("VKPP_ENABLE_VALIDATION_LAYER");
    if (!envEnableValidation.empty())
    {
        if ((std::atoi(envEnableValidation.c_str()) == 1) ||
            rad::StrCaseEqual(envEnableValidation, "true"))
        {
            enableValidation = true;
        }
        else
        {
            enableValidation = false;
        }
    }

    if (enableValidation)
    {
        if (HasLayer(layers, "VK_LAYER_KHRONOS_validation"))
        {
            enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
        }
        else
        {
            RAD_LOG(GetLogger(), warn, "Cannot enable validation layer: "
                "VK_LAYER_KHRONOS_validation not available!");
            enableValidation = false;
        }
    }

    std::vector<VkExtensionProperties> extensions = EnumerateInstanceExtensions(nullptr);

    if (HasExtension(extensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
    {
        m_enabledExtensions.insert(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    if (HasExtension(extensions, VK_KHR_SURFACE_EXTENSION_NAME))
    {
        m_enabledExtensions.insert(VK_KHR_SURFACE_EXTENSION_NAME);
    }
#if defined(RAD_OS_WINDOWS)
    if (HasExtension(extensions, VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
    {
        m_enabledExtensions.insert(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    }
#endif
    if (HasExtension(extensions, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME))
    {
        m_enabledExtensions.insert(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    }

    if (enableValidation)
    {
        if (HasExtension(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
        {
            m_enabledExtensions.insert(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else
        {
            RAD_LOG(GetLogger(), warn, "Cannot enable validation layer: {} not available!",
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            enableValidation = false;
        }
    }

    std::vector<const char*> enabledExtensions;
    for (const std::string& extension : m_enabledExtensions)
    {
        enabledExtensions.push_back(extension.data());
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = appName.data();
    appInfo.applicationVersion = appVersion;
    appInfo.pEngineName = appName.data();
    appInfo.engineVersion = appVersion;
    appInfo.apiVersion = m_version;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.flags = 0;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};
    if (enableValidation)
    {
        debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugMessengerCreateInfo.pNext = nullptr;
        debugMessengerCreateInfo.flags = 0;
        debugMessengerCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugMessengerCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
        debugMessengerCreateInfo.pfnUserCallback = DebugUtilsMessengerCallback;
        debugMessengerCreateInfo.pUserData = nullptr;
        instanceCreateInfo.pNext = &debugMessengerCreateInfo;
    }

    VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &m_handle));
    if (m_handle)
    {
        volkLoadInstance(m_handle);
    }

    if (enableValidation)
    {
        VK_CHECK(vkCreateDebugUtilsMessengerEXT(m_handle,
            &debugMessengerCreateInfo, nullptr, &m_debugMessenger));
    }

    return true;
}

bool Instance::IsExtensionSupported(std::string_view name) const
{
    return m_enabledExtensions.contains(name);
}

std::vector<rad::Ref<PhysicalDevice>> Instance::EnumeratePhysicalDevices()
{
    std::vector<rad::Ref<PhysicalDevice>> devices;
    uint32_t deviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(m_handle, &deviceCount, nullptr));
    if (deviceCount > 0)
    {
        std::vector<VkPhysicalDevice> deviceHandles(deviceCount);
        VK_CHECK(vkEnumeratePhysicalDevices(m_handle, &deviceCount, deviceHandles.data()));
        devices.resize(deviceCount);
        for (uint32_t i = 0; i < deviceCount; ++i)
        {
            devices[i] = RAD_NEW PhysicalDevice(this, deviceHandles[i]);
        }
    }
    return devices;
}

} // namespace vkpp
