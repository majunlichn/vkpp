#pragma once

#include <vkpp/Core/Context.h>
#include <sdlpp/Gui/Window.h>

namespace vkpp
{

class Window : public sdl::Window
{
public:
    Window(rad::Ref<Context> context);
    ~Window();

    bool Create(const char* title, int w, int h, SDL_WindowFlags flags);

    Surface* GetSurface() { return m_surface.get(); }
    ImageView* GetRenderTargetView() { return m_renderTargetView.get(); }
    ImageView* GetOverlayView() { return m_overlayView.get(); }
    Swapchain* GetSwapchain() { return m_swapchain.get(); }
    size_t GetFrameIndex() const { return m_frameIndex; }

    void BeginFrame();
    void EndFrame();

protected:
    virtual void OnResized(int width, int height) override;
    virtual void OnDestroyed() override;

    void Resize(int width, int height);
    void Resize();

    rad::Ref<Context> m_context;
    rad::Ref<Surface> m_surface;

private:
    rad::Ref<Surface> CreateSurface();
    bool CreateSamplers();
    bool CreateSwapchain(uint32_t width, uint32_t height);
    bool CreateBlitPipeline();
    void UpdateBlitBindings();
    void BlitToSwapchain();
    void Present();

    VkSurfaceCapabilitiesKHR m_surfaceCaps = {};
    std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
    std::vector<VkPresentModeKHR> m_presentModes;
    VkSurfaceFormatKHR m_surfaceFormat = {};
    VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;

    rad::Ref<Image> m_renderTarget;
    rad::Ref<ImageView> m_renderTargetView;
    rad::Ref<Image> m_overlay;
    rad::Ref<ImageView> m_overlayView;
    rad::Ref<Sampler> m_samplerNearest;
    rad::Ref<Sampler> m_samplerLinear;

    // Prefer triple-buffering: 3-long first in, first out queue.
    // https://en.wikipedia.org/wiki/Multiple_buffering
    uint32_t m_presentBufferCount = 3;
    rad::Ref<Swapchain> m_swapchain;

    struct BlitToSwapchain
    {
        rad::Ref<DescriptorSetLayout> descSetLayout;
        rad::Ref<PipelineLayout> pipelineLayout;
        rad::Ref<Pipeline> pipeline;
    } m_blit;

    rad::Ref<DescriptorPool> m_descPool;
    rad::Ref<DescriptorSet> m_descSet;

    rad::Ref<CommandPool> m_cmdPool;
    std::vector<rad::Ref<CommandBuffer>> m_cmdBuffers;
    uint32_t m_cmdBufferIndex = 0;

    size_t m_frameIndex = 0;
    // Allow a maximum of two outstanding presentation operations.
    static const uint32_t MaxFrameLag = 2;
    rad::Ref<Semaphore> m_swapchainImageAcquired[MaxFrameLag];
    rad::Ref<Semaphore> m_drawComplete[MaxFrameLag];
    rad::Ref<Semaphore> m_swapchainImageOwnershipTransferComplete[MaxFrameLag];
    // Fences that we can use to throttle if we get too far ahead of image presents.
    rad::Ref<Fence> m_fences[MaxFrameLag];

}; // class Window

} // namespace vkpp
