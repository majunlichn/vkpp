#pragma once

#include <vkpp/Gui/Window.h>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>

namespace vkpp
{

class GuiRenderer : public rad::RefCounted<GuiRenderer>
{
public:
    GuiRenderer(Window* window);
    ~GuiRenderer();

    ImGuiIO& GetIO() { return ImGui::GetIO(); }
    ImFontAtlas* GetFonts() { return ImGui::GetIO().Fonts; }

    bool Init(rad::Span<VkDescriptorPoolSize> descPoolSizes = {});
    void Destroy();

    Image* GetRenderTarget() { return m_renderTarget.get(); }
    ImageView* GetRenderTargetView() { return m_renderTargetView.get(); }

    bool ProcessEvent(const SDL_Event& event);
    void NewFrame();
    void Render();

private:
    Window* m_window;
    ImGuiContext* m_gui = nullptr;

    VkFormat m_renderTargetFormat = VK_FORMAT_R8G8B8A8_UNORM;
    rad::Ref<Image> m_renderTarget;
    rad::Ref<ImageView> m_renderTargetView;

    rad::Ref<DescriptorPool> m_descPool;

    rad::Ref<CommandPool> m_cmdPool;
    std::vector<rad::Ref<CommandBuffer>> m_cmdBuffers;
    uint32_t m_cmdBufferIndex = 0;
    VkClearColorValue m_clearValue = {};

}; // class GuiRenderer

} // namespace vkpp
