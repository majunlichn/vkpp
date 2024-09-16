#include "VulkanViewer.h"

VulkanViewer::VulkanViewer(rad::Ref<vkpp::Context> context) :
    vkpp::Window(context)
{
    m_logger = rad::CreateLogger("VukanViewer");
}

VulkanViewer::~VulkanViewer()
{
}

bool VulkanViewer::Init()
{
    if (!vkpp::Window::Create("VulkanViewer", 800, 600,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN))
    {
        return false;
    }
    return true;
}

bool VulkanViewer::OnEvent(const SDL_Event& event)
{
    return vkpp::Window::OnEvent(event);
}

void VulkanViewer::OnIdle()
{
    if (GetFlags() & SDL_WINDOW_MINIMIZED)
    {
        return;
    }

    BeginFrame();
    // TODO: draw something?
    EndFrame();
}
