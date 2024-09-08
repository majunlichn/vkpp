#include <vkpp/Core/Swapchain.h>
#include <vkpp/Core/Device.h>
#include <vkpp/Core/Image.h>
#include <vkpp/Core/Fence.h>
#include <vkpp/Core/Semaphore.h>
#include <vkpp/Core/Surface.h>

namespace vkpp
{

Swapchain::Swapchain(
    rad::Ref<Device> device,
    rad::Ref<Surface> surface,
    const VkSwapchainCreateInfoKHR& createInfo) :
    m_device(std::move(device)),
    m_surface(std::move(surface))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateSwapchainKHR(m_device->GetHandle(), &createInfo, nullptr, &m_handle));

    m_imageCount = createInfo.minImageCount;
    m_format = createInfo.imageFormat;
    m_width = createInfo.imageExtent.width;
    m_height = createInfo.imageExtent.height;
    m_presentMode = createInfo.presentMode;

    std::vector<VkImage> m_imageHandles(m_imageCount);
    VK_CHECK(m_device->GetFunctionTable()->
        vkGetSwapchainImagesKHR(m_device->GetHandle(), m_handle, &m_imageCount, nullptr));
    if (m_imageCount > 0)
    {
        VK_CHECK(m_device->GetFunctionTable()->
            vkGetSwapchainImagesKHR(m_device->GetHandle(), m_handle,
                &m_imageCount, m_imageHandles.data()));
    }

    m_images.resize(m_imageCount);
    m_imageViews.resize(m_imageCount);
    for (uint32_t i = 0; i < m_imageCount; ++i)
    {
        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.pNext = nullptr;
        imageCreateInfo.flags = 0;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = createInfo.imageFormat;
        imageCreateInfo.extent.width = createInfo.imageExtent.width;
        imageCreateInfo.extent.height = createInfo.imageExtent.height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = createInfo.imageArrayLayers;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = createInfo.imageUsage;
        imageCreateInfo.sharingMode = createInfo.imageSharingMode;
        imageCreateInfo.queueFamilyIndexCount = createInfo.queueFamilyIndexCount;
        imageCreateInfo.pQueueFamilyIndices = createInfo.pQueueFamilyIndices;
        imageCreateInfo.initialLayout;
        m_images[i] = RAD_NEW Image(m_device, imageCreateInfo, m_imageHandles[i]);
        m_imageViews[i] = m_images[i]->CreateDefaultView();
    }
}

Swapchain::~Swapchain()
{
    m_device->GetFunctionTable()->
        vkDestroySwapchainKHR(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}

uint32_t Swapchain::GetImageCount() const
{
    return m_imageCount;
}

VkFormat Swapchain::GetFormat() const
{
    return m_format;
}

uint32_t Swapchain::GetWidth() const
{
    return m_width;
}

uint32_t Swapchain::GetHeight() const
{
    return m_height;
}

VkPresentModeKHR Swapchain::GetPresentMode() const
{
    return m_presentMode;
}

Image* Swapchain::GetImage(uint32_t index)
{
    return m_images[index].get();
}

ImageView* Swapchain::GetDefaultView(uint32_t index)
{
    return m_imageViews[index].get();
}

Image* Swapchain::GetCurrentImage()
{
    return m_images[m_currentImageIndex].get();
}

ImageView* Swapchain::GetCurrentImageView()
{
    return m_imageViews[m_currentImageIndex].get();
}

uint32_t Swapchain::GetCurrentImageIndex() const
{
    return m_currentImageIndex;
}

VkResult Swapchain::AcquireNextImage(
    Semaphore* semaphore, Fence* fence, uint64_t timeout)
{
    VkSemaphore semaphoreHandle = semaphore ? semaphore->GetHandle() : VK_NULL_HANDLE;
    VkFence fenceHandle = fence ? fence->GetHandle() : VK_NULL_HANDLE;
    return m_device->GetFunctionTable()->
        vkAcquireNextImageKHR(m_device->GetHandle(), m_handle,
            timeout, semaphoreHandle, fenceHandle, &m_currentImageIndex);
}

} // namespace vkpp
