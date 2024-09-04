#include <vkpp/Core/Common.h>
#include <rad/System/Application.h>

int main(int argc, char* argv[])
{
    rad::Application app;
    if (!app.Init(argc, argv))
    {
        RAD_LOG_DEFAULT(err, "rad::Application::Init failed!");
        return -1;
    }

    VkResult result = volkInitialize();
    if (result == VK_SUCCESS)
    {
        uint32_t version = volkGetInstanceVersion();
        RAD_LOG(vkpp::GetLogger(), info, "Vulkan Instance Version: {}.{}.{}",
            VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));
    }
    else
    {
        RAD_LOG(vkpp::GetLogger(), err, "volkInitialize failed!");
    }

    volkFinalize();

    return 0;
}
