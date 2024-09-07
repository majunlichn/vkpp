#include <vkpp/Core/Instance.h>
#include <vkpp/Core/PhysicalDevice.h>
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
        RAD_LOG_DEFAULT(info, "Vulkan instance initialized.");
    }

    auto gpus = instance->EnumeratePhysicalDevices();
    for (size_t i = 0; i < gpus.size(); ++i)
    {
        RAD_LOG_DEFAULT(info, "Vulkan device#{}: {}", i, gpus[i]->GetDeviceName());
    }

    return 0;
}
