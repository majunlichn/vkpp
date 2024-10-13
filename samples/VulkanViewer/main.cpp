#define SDL_MAIN_USE_CALLBACKS
#include "VulkanViewer.h"
#include <SDL3/SDL_main.h>

rad::Ref<sdl::Application> g_app;
rad::Ref<vkpp::Context> g_context;
rad::Ref<VulkanViewer> g_viewer;

SDL_AppResult SDL_AppInit(void** appState, int argc, char** argv)
{
    g_app = RAD_NEW sdl::Application();
    if (!g_app->Init(argc, argv))
    {
        RAD_LOG_DEFAULT(err, "sdl::Application::Init failed!");
        return SDL_APP_FAILURE;
    }
    rad::Ref<vkpp::Instance> instance = RAD_NEW vkpp::Instance();
    if (!instance->Init("VulkanViewer", VK_MAKE_VERSION(0, 0, 0)))
    {
        VKPP_LOG(err, "vkpp::Instance::Init failed!");
        return SDL_APP_FAILURE;
    }
    g_context = RAD_NEW vkpp::Context();
    if (!g_context->Init(instance, nullptr))
    {
        VKPP_LOG(err, "vkpp::Context::Init failed!");
        return SDL_APP_FAILURE;
    }
    g_viewer = RAD_NEW VulkanViewer(g_context);
    if (!g_viewer->Init())
    {
        RAD_LOG(g_viewer->GetLogger(), err, "VulkanViewer::Init failed!");
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appState)
{
    g_app->OnIdle();
    return g_app->GetExit() ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appState, SDL_Event* event)
{
    g_app->OnEvent(*event);
    return g_app->GetExit() ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    g_viewer.reset();
    g_context.reset();
    g_app.reset();
}
