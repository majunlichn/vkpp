#include <vkpp/Core/Context.h>
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
    if (!instance->Init("VulkanViewer", VK_MAKE_VERSION(0, 0, 0)))
    {
        VKPP_LOG(err, "vkpp::Instance::Init failed!");
        return -1;
    }

    rad::Ref<vkpp::Context> context = RAD_NEW vkpp::Context();
    if (!context->Init(instance, nullptr))
    {
        RAD_LOG_DEFAULT(err, "vkpp::Context::Init failed!");
        return -1;
    }
    return 0;
}
