#pragma once

#include <vkpp/Gui/Window.h>

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

private:
    std::shared_ptr<spdlog::logger> m_logger;

}; // class VulkanViewer
