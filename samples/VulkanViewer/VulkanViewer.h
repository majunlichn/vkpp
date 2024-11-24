#pragma once

#include <vkpp/Gui/Window.h>
#include <vkpp/Gui/GuiRenderer.h>
#include <vkpp/Scene/Scene.h>
#include <vkpp/Rendering/SolidRenderer.h>
#include <vkpp/Scene/CameraController.h>

class VulkanViewer : public vkpp::Window
{
public:
    VulkanViewer(rad::Ref<vkpp::Context> context);
    ~VulkanViewer();

    spdlog::logger* GetLogger() { return m_logger.get(); }
    bool Init();

protected:
    virtual bool OnEvent(const SDL_Event& event) override;
    virtual void OnIdle() override;

    virtual void OnResized(int width, int height) override;
    void Resize(int width, int height);

    virtual void OnKeyDown(const SDL_KeyboardEvent& keyDown) override;
    virtual void OnKeyUp(const SDL_KeyboardEvent& keyUp) override;

    virtual void OnMouseMove(const SDL_MouseMotionEvent& mouseMotion) override;

private:
    std::shared_ptr<spdlog::logger> m_logger;
    rad::Ref<vkpp::GuiRenderer> m_guiRenderer;
    bool m_showDemoWindow = true;

    rad::Ref<vkpp::Scene> m_scene;
    rad::Ref<vkpp::CameraController> m_cameraController;
    vkpp::CameraController::Input m_cameraInput = {};
    float m_yawRelToRadians = 1.0f;
    float m_pitchRelToRadians = 1.0f;
    rad::Ref<vkpp::SolidRenderer> m_solidRenderer;

}; // class VulkanViewer
