#include "VulkanViewer.h"
#include <vkpp/Rendering/SceneImporter.h>

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
    if (!vkpp::Window::Create("VulkanViewer", 1600, 900,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN))
    {
        return false;
    }

    m_gui = RAD_NEW vkpp::GuiContext(this);
    if (!m_gui->Init())
    {
        RAD_LOG(m_logger, err, "m_gui->Init() failed!");
        return false;
    }

    if (sdl::GetApp()->GetArgc() > 1)
    {
        m_scene = RAD_NEW vkpp::Scene(m_context);
        rad::Ref<vkpp::SceneImporter> importer = RAD_NEW vkpp::SceneImporter(m_scene.get());
        std::string fileName = sdl::GetApp()->GetArgv()[1];
        if (importer->Import(fileName))
        {
            RAD_LOG(m_logger, info, "Scene imported successfully: {}", fileName);
        }
        else
        {
            RAD_LOG(m_logger, err, "Failed to import {}!!", fileName);
        }
    }

    return true;
}

bool VulkanViewer::OnEvent(const SDL_Event& event)
{
    if (m_gui)
    {
        m_gui->ProcessEvent(event);
    }
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
    m_gui->NewFrame();
    if (m_showDemoWindow)
    {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }
    m_gui->Render();
    EndFrame();
}

void VulkanViewer::OnResized(int width, int height)
{
    Resize(width, height);
}

void VulkanViewer::Resize(int width, int height)
{
    vkpp::Window::Resize(width, height);
    m_gui = RAD_NEW vkpp::GuiContext(this);
    if (!m_gui->Init())
    {
        RAD_LOG(m_logger, err, "m_gui->Init() failed!");
    }
}

void VulkanViewer::OnKeyDown(const SDL_KeyboardEvent& keyDown)
{
    RAD_LOG(m_logger, info, "OnKeyDown: {}", SDL_GetKeyName(keyDown.key));
    if (keyDown.key == SDLK_F1)
    {
        m_showDemoWindow = !m_showDemoWindow;
    }
}

void VulkanViewer::OnKeyUp(const SDL_KeyboardEvent& keyUp)
{
    RAD_LOG(m_logger, info, "OnKeyUp: {}", SDL_GetKeyName(keyUp.key));
}
