#pragma once

#include <vkpp/Gui/Window.h>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>

namespace vkpp
{

class GuiContext : public rad::RefCounted<GuiContext>
{
public:
    GuiContext(Window* window);
    ~GuiContext();

    ImGuiIO& GetIO() { return ImGui::GetIO(); }
    ImFontAtlas* GetFonts() { return ImGui::GetIO().Fonts; }

    bool Init(rad::Span<VkDescriptorPoolSize> poolSizes = {});
    void Destroy();

    bool ProcessEvent(const SDL_Event& event);
    void NewFrame();
    void Render();

private:
    Window* m_window;
    ImGuiContext* m_gui = nullptr;

    rad::Ref<DescriptorPool> m_descPool;

    rad::Ref<CommandPool> m_cmdPool;
    std::vector<rad::Ref<CommandBuffer>> m_cmdBuffers;
    uint32_t m_cmdBufferIndex = 0;
    VkClearColorValue m_clearValue = {};

}; // class GuiContext

} // namespace vkpp
