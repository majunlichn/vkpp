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

    m_solidRenderer = RAD_NEW vkpp::SolidRenderer(m_context);
    if (!m_solidRenderer->Init())
    {
        RAD_LOG(m_logger, err, "Failed to create the SolidRenderer!");
        return false;
    }

    SetPresentViews(
        m_solidRenderer->m_renderTargetView,
        m_gui->GetRenderTargetView());

    if (sdl::GetApp()->GetArgc() > 1)
    {
        m_scene = RAD_NEW vkpp::Scene(m_context);
        rad::Ref<vkpp::SceneImporter> importer = RAD_NEW vkpp::SceneImporter(m_scene.get());
        std::string fileName = sdl::GetApp()->GetArgv()[1];
        if (importer->Import(fileName))
        {
            int width = 0;
            int height = 0;
            GetSizeInPixels(&width, &height);
            m_scene->m_camera->m_aspectRatio = float(width) / float(height);
            m_scene->SetCameraFrontView();
            m_scene->Upload();
            RAD_LOG(m_logger, info, "Scene imported successfully: {}", fileName);
            m_solidRenderer->LoadScene(m_scene.get());
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
    static Uint64 lastTime = SDL_GetTicksNS();
    Uint64 currTime = SDL_GetTicksNS();
    float deltaTime = float(currTime - lastTime) / 1e9f;
    lastTime = currTime;

    if (GetFlags() & SDL_WINDOW_MINIMIZED)
    {
        return;
    }

    BeginFrame();

    if (m_scene)
    {
        if (m_cameraController)
        {
            m_cameraController->Update(deltaTime);
        }

        if (m_solidRenderer)
        {
            m_solidRenderer->Render();
        }
    }

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
    GetSizeInPixels(&width, &height);
    Resize(width, height);
}

void VulkanViewer::Resize(int width, int height)
{
    m_context->WaitIdle();

    if (m_scene)
    {
        m_scene->m_camera->m_aspectRatio = float(width) / float(height);
    }

    vkpp::Window::Resize(width, height);

    m_solidRenderer->Resize(width, height);

    m_gui = RAD_NEW vkpp::GuiContext(this);
    if (!m_gui->Init())
    {
        RAD_LOG(m_logger, err, "m_gui->Init() failed!");
    }

    SetPresentViews(
        m_solidRenderer->m_renderTargetView, m_gui->GetRenderTargetView());
}

void VulkanViewer::OnKeyDown(const SDL_KeyboardEvent& keyDown)
{
    RAD_LOG(m_logger, info, "OnKeyDown: {}", SDL_GetKeyName(keyDown.key));

    if (keyDown.key == SDLK_F1)
    {
        if (m_cameraController)
        {
            m_cameraController.reset();
            int width = 0;
            int height = 0;
            GetSizeInPixels(&width, &height);
            SDL_WarpMouseInWindow(m_handle, width / 2.0f, height / 2.0f);
            SDL_SetWindowRelativeMouseMode(m_handle, false);
        }
        else if (m_scene)
        {
            m_cameraController = RAD_NEW vkpp::CameraController(m_scene->m_camera.get());
            glm::vec3 diagonal = m_scene->GetBoundingBox().GetDiagonal();
            m_cameraController->SetMoveAroundSpeed(std::max(diagonal.x, diagonal.y) / 4.0f);
            m_cameraController->SetMoveVerticalSpeed(diagonal.z / 4.0f);

            vkpp::CameraController::Input& input = m_cameraController->m_input;
            SDL_DisplayID displayID = SDL_GetDisplayForWindow(m_handle);
            SDL_Rect rect = {};
            SDL_GetDisplayBounds(displayID, &rect);
            input.yawRelToRadians = glm::pi<float>() / (float(rect.w) / 60.0f);
            input.pitchRelToRadians = glm::pi<float>() / (float(rect.h) / 60.0f);

            SDL_SetWindowRelativeMouseMode(m_handle, true);
        }
    }

    if (m_cameraController)
    {
        vkpp::CameraController::Input& input = m_cameraController->m_input;
        if (keyDown.key == SDLK_W)
        {
            input.moveForward = true;
        }
        if (keyDown.key == SDLK_S)
        {
            input.moveBack = true;
        }
        if (keyDown.key == SDLK_A)
        {
            input.moveLeft = true;
        }
        if (keyDown.key == SDLK_D)
        {
            input.moveRight = true;
        }
        if (keyDown.key == SDLK_Q)
        {
            input.moveUp = true;
        }
        if (keyDown.key == SDLK_E)
        {
            input.moveDown = true;
        }
    }

    if (keyDown.key == SDLK_F12)
    {
        m_showDemoWindow = !m_showDemoWindow;
    }
}

void VulkanViewer::OnKeyUp(const SDL_KeyboardEvent& keyUp)
{
    RAD_LOG(m_logger, info, "OnKeyUp: {}", SDL_GetKeyName(keyUp.key));

    if (m_cameraController)
    {
        vkpp::CameraController::Input& input = m_cameraController->m_input;
        if (keyUp.key == SDLK_W)
        {
            input.moveForward = false;
        }
        if (keyUp.key == SDLK_S)
        {
            input.moveBack = false;
        }
        if (keyUp.key == SDLK_A)
        {
            input.moveLeft = false;
        }
        if (keyUp.key == SDLK_D)
        {
            input.moveRight = false;
        }
        if (keyUp.key == SDLK_Q)
        {
            input.moveUp = false;
        }
        if (keyUp.key == SDLK_E)
        {
            input.moveDown = false;
        }
    }
}

void VulkanViewer::OnMouseMove(const SDL_MouseMotionEvent& mouseMotion)
{
    float dx = mouseMotion.xrel;
    float dy = mouseMotion.yrel;
    if (m_cameraController)
    {
        vkpp::CameraController::Input& input = m_cameraController->m_input;
        input.yawRel += -dx;
        input.pitchRel += -dy;
    }
}
