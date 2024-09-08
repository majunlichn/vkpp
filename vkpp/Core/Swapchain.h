#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class Swapchain : public rad::RefCounted<Swapchain>
{
public:
    Swapchain(
        rad::Ref<Device> device,
        rad::Ref<Surface> surface,
        const VkSwapchainCreateInfoKHR& createInfo);
    ~Swapchain();
    VKPP_DISABLE_COPY_AND_MOVE(Swapchain);

    VkSwapchainKHR GetHandle() const { return m_handle; }

    uint32_t GetImageCount() const;
    VkFormat GetFormat() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    VkPresentModeKHR GetPresentMode() const;

    Image* GetImage(uint32_t index);
    ImageView* GetDefaultView(uint32_t index);
    Image* GetCurrentImage();
    ImageView* GetCurrentImageView();

    uint32_t GetCurrentImageIndex() const;
    // @param timeout: indicates how long the function waits, in nanoseconds, if no image is available.
    VkResult AcquireNextImage(
        Semaphore* semaphore, Fence* fence = nullptr, uint64_t timeout = UINT64_MAX);

private:
    rad::Ref<Device>            m_device;
    rad::Ref<Surface>           m_surface;
    VkSwapchainKHR              m_handle = VK_NULL_HANDLE;
    VkFormat                    m_format;
    uint32_t                    m_imageCount;
    uint32_t                    m_width;
    uint32_t                    m_height;
    VkPresentModeKHR            m_presentMode;
    std::vector<rad::Ref<Image>> m_images;
    std::vector<rad::Ref<ImageView>> m_imageViews;
    uint32_t                    m_currentImageIndex = 0;

}; // class Swapchain

} // namespace vkpp
