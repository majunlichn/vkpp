#include <vkpp/Gui/Window.h>
#include <SDL3/SDL_vulkan.h>
#include <rad/Core/Flags.h>

namespace vkpp
{

Window::Window(rad::Ref<Context> context) :
    m_context(std::move(context))
{
}

Window::~Window()
{
    Device* device = m_context->GetDevice();
    device->WaitIdle();
    sdl::Window::Destroy();
}

bool Window::Create(const char* title, int w, int h, SDL_WindowFlags flags)
{
    if (sdl::Window::Create(title, w, h, flags | SDL_WINDOW_VULKAN))
    {
        if (m_surface = CreateSurface())
        {
            Resize();
            return true;
        }
        else
        {
            VKPP_LOG(err, "Failed to create window surface!");
            return false;
        }
    }
    else
    {
        return false;
    }
}

rad::Ref<Surface> Window::CreateSurface()
{
    VkSurfaceKHR surfaceHandle = VK_NULL_HANDLE;
    Instance* instance = m_context->GetInstance();
    bool result = SDL_Vulkan_CreateSurface(
        m_handle, instance->GetHandle(), nullptr, &surfaceHandle);
    if (result == true)
    {
        return RAD_NEW Surface(m_context->GetInstance(), surfaceHandle);
    }
    else
    {
        VKPP_LOG(err, "SDL_Vulkan_CreateSurface failed: {}",
            SDL_GetError());
        return nullptr;
    }
}

void Window::BeginFrame()
{
    // Ensure no more than MaxFrameLag renderings are outstanding.
    m_fences[m_backBufferIndex]->Wait();
    m_fences[m_backBufferIndex]->Reset();

    VkResult result = VK_SUCCESS;
    do {
        // If both back buffers has been drawn, waits until the first one is placed on the screen.
        result = m_swapchain->AcquireNextImage(
            m_swapchainImageAcquired[m_backBufferIndex].get(), nullptr);
        if (result > 0)
        {
            VKPP_LOG(info, "swapchain->AcquireNextImage returns {}", string_VkResult(result));
            if (result == VK_SUBOPTIMAL_KHR)
            {
                // swapchain is not as optimal as it could be, but the platform's
                // presentation engine will still present the image correctly.
                break;
            }
        }
        else if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            Resize();
            break;
        }
        else if (result == VK_ERROR_SURFACE_LOST_KHR)
        {
            m_surface = CreateSurface(); // recreate the surface.
            Resize();
            break;
        }
        else if (result < 0)
        {
            VKPP_LOG(err, "swapchain->AcquireNextImage failed: {}", string_VkResult(result));
            throw Error(result);
        }
    } while (result != VK_SUCCESS);
}

void Window::EndFrame()
{
    BlitToSwapchain();
    Present();

    m_frameIndex += 1;

    m_backBufferIndex += 1;
    m_backBufferIndex %= MaxFrameLag;

    m_cmdBufferIndex += 1;
    m_cmdBufferIndex %= m_cmdBuffers.size();
}

void Window::OnResized(int width, int height)
{
    Resize(width, height);
}

void Window::OnDestroyed()
{
    Device* device = m_context->GetDevice();
    device->WaitIdle();
}

void Window::Resize(int width, int height)
{
    Device* device = m_context->GetDevice();
    Queue* queue = m_context->GetQueue();

    if (!device->IsSurfaceSupported(queue->GetQueueFamily(), m_surface.get()))
    {
        VKPP_LOG(err, "GPU \"{}\" doesn't support the window surface {}  (ID={}, Title=\"{}\")!",
            device->GetDeviceName(), (void*)GetHandle(),
            GetID(), GetTitle());
        return;
    }

    m_surfaceCaps = device->GetPhysicalDevice()->GetSurfaceCapabilities(m_surface->GetHandle());
    m_surfaceFormats = device->GetPhysicalDevice()->GetSurfaceFormats(m_surface->GetHandle());
    m_presentModes = device->GetPhysicalDevice()->GetSurfacePresentModes(m_surface->GetHandle());

    auto& swapchainImageCount = m_context->m_swapchainImageCount;
    if (swapchainImageCount < m_surfaceCaps.minImageCount)
    {
        swapchainImageCount = m_surfaceCaps.minImageCount;
    }
    if ((m_surfaceCaps.maxImageCount > 0) &&
        (swapchainImageCount > m_surfaceCaps.maxImageCount))
    {
        swapchainImageCount = m_surfaceCaps.maxImageCount;
    }

    if (!m_renderTarget)
    {
        // TODO: render scale?
        m_renderTarget = device->CreateImage2DRenderTarget(
            m_context->m_colorFormat, width, height, VK_IMAGE_USAGE_SAMPLED_BIT);
    }
    if (!m_renderTargetView)
    {
        m_renderTargetView = m_renderTarget->CreateDefaultView();
    }

    m_overlay = device->CreateImage2DRenderTarget(
        VK_FORMAT_R8G8B8A8_UNORM, width, height, VK_IMAGE_USAGE_SAMPLED_BIT);
    m_overlayView = m_overlay->CreateDefaultView();

    CreateSwapchain(width, height);
    CreateSamplers();

    CreateBlitPipeline();
    UpdateBlitBindings();

    m_cmdPool = device->CreateCommandPool(queue->GetQueueFamily(),
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    m_cmdBuffers.resize(swapchainImageCount);
    for (uint32_t i = 0; i < m_cmdBuffers.size(); ++i)
    {
        m_cmdBuffers[i] = m_cmdPool->Allocate();
    }

    // Init synchronization primitives.
    for (size_t i = 0; i < MaxFrameLag; ++i)
    {
        m_swapchainImageAcquired[i] = device->CreateSemaphore();
        m_drawComplete[i] = device->CreateSemaphore();
        m_fences[i] = device->CreateFence(VK_FENCE_CREATE_SIGNALED_BIT);
    }
}

void Window::Resize()
{
    int width = 0;
    int height = 0;
    GetSizeInPixels(&width, &height);
    Resize(width, height);
    m_context->m_resolution.width = width;
    m_context->m_resolution.height = height;
}

bool Window::CreateSamplers()
{
    Device* device = m_context->GetDevice();
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.mipLodBias = 0;
    samplerInfo.minLod = 0;
    samplerInfo.maxLod = 0;
    m_samplerNearest = device->CreatSampler(samplerInfo);
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    m_samplerLinear = device->CreatSampler(samplerInfo);
    return true;
}

bool Window::CreateSwapchain(uint32_t width, uint32_t height)
{
    Device* device = m_context->GetDevice();

    // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
    if (m_surfaceCaps.currentExtent.width == 0xFFFFFFFF)
    {
        // If the surface size is undefined, the size is set to the size of the images requested,
        // which must fit within the minimum and maximum values.
        if (width < m_surfaceCaps.minImageExtent.width)
        {
            width = m_surfaceCaps.minImageExtent.width;
        }
        else if (width > m_surfaceCaps.maxImageExtent.width)
        {
            width = m_surfaceCaps.maxImageExtent.width;
        }

        if (height < m_surfaceCaps.minImageExtent.height)
        {
            height = m_surfaceCaps.minImageExtent.height;
        }
        else if (height > m_surfaceCaps.minImageExtent.height)
        {
            height = m_surfaceCaps.minImageExtent.height;
        }
    }
    else
    {
        // If the surface size is defined, the swap chain size must match
        width = m_surfaceCaps.minImageExtent.width;
        height = m_surfaceCaps.minImageExtent.height;
    }

    if (width == 0 || height == 0)
    {
        return false;
    }

    uint32_t imageCount = 3;
    if (imageCount < m_surfaceCaps.minImageCount)
    {
        imageCount = m_surfaceCaps.minImageCount;
    }
    if ((m_surfaceCaps.maxImageCount > 0) &&
        (imageCount > m_surfaceCaps.maxImageCount))
    {
        imageCount = m_surfaceCaps.maxImageCount;
    }

    m_surfaceFormat = m_surfaceFormats[0];
    if (m_surfaceFormat.format == VK_FORMAT_UNDEFINED)
    {
        m_surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    }

    if (vkuFormatElementSize(m_context->m_colorFormat) >= 32)
    {
        for (size_t i = 0; i < m_surfaceFormats.size(); i++)
        {
            const VkFormat format = m_surfaceFormats[i].format;
            if ((format == VK_FORMAT_A2R10G10B10_UNORM_PACK32) ||
                (format == VK_FORMAT_A2B10G10R10_UNORM_PACK32) ||
                (format == VK_FORMAT_R16G16B16A16_SFLOAT))
            {
                m_surfaceFormat = m_surfaceFormats[i];
                break;
            }
        }
    }

    VKPP_LOG(info, "Select SurfaceFormat {} ({})",
        string_VkFormat(m_surfaceFormat.format),
        string_VkColorSpaceKHR(m_surfaceFormat.colorSpace));

    VkColorSpaceKHR colorSpace = m_surfaceFormats[0].colorSpace;

    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    if (m_surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        preTransform = m_surfaceCaps.currentTransform;
    }

    rad::Flags32<VkCompositeAlphaFlagBitsKHR> supportedCompositeAlpha(m_surfaceCaps.supportedCompositeAlpha);
    VkCompositeAlphaFlagBitsKHR compositeAlpha =
        rad::HasBits<uint32_t>(m_surfaceCaps.supportedCompositeAlpha, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (supportedCompositeAlpha.HasBits(VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR))
    {
        compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
    }
    else if (supportedCompositeAlpha.HasBits(VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR))
    {
        compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    }

    if (std::ranges::find(m_presentModes, m_presentMode) == std::end(m_presentModes))
    {
        VKPP_LOG(warn, "PresentMode {} is not supported, fallback to {}!\n",
            string_VkPresentModeKHR(m_presentMode),
            string_VkPresentModeKHR(m_presentModes[0]));
        m_presentMode = m_presentModes[0];
    }

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = nullptr;
    swapchainInfo.flags = 0;
    swapchainInfo.surface = m_surface->GetHandle();
    swapchainInfo.minImageCount = imageCount;
    swapchainInfo.imageFormat = m_surfaceFormat.format;
    swapchainInfo.imageColorSpace = colorSpace;
    swapchainInfo.imageExtent.width = width;
    swapchainInfo.imageExtent.height = height;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.queueFamilyIndexCount = 0;
    swapchainInfo.pQueueFamilyIndices = nullptr;
    swapchainInfo.preTransform = preTransform;
    swapchainInfo.compositeAlpha = compositeAlpha;
    swapchainInfo.presentMode = m_presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = m_swapchain ? m_swapchain->GetHandle() : VK_NULL_HANDLE;

    m_swapchain = RAD_NEW Swapchain(device, m_surface, swapchainInfo);
    VKPP_LOG(info, "VulkanSwapchain (re)created: extent={}x{};",
        m_swapchain->GetWidth(), m_swapchain->GetHeight());
    return true;
}

bool Window::CreateBlitPipeline()
{
    Device* device = m_context->GetDevice();

    std::vector<VkDescriptorSetLayoutBinding> descBindings =
    { // binding, type, count, stageFlags, samplers
        { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }, // renderTarget
        { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }, // overlay
    };
    m_blitDescSetLayout = device->CreateDescriptorSetLayout(descBindings);
    m_blitPipelineLayout = device->CreatePipelineLayout(m_blitDescSetLayout.get());

    rad::Ref<GraphicsPipelineCreateInfo> pipelineInfo =
        RAD_NEW GraphicsPipelineCreateInfo(device);
    uint32_t vertBinary[] =
    {
#include <vkpp/Shaders/Compiled/Gui/BlitToSwapchain.vert.inc>
    };
    uint32_t fragBinary[] =
    {
#include <vkpp/Shaders/Compiled/Gui/BlitToSwapchain.frag.inc>
    };
    rad::Ref<ShaderModule> vert = device->CreateShaderModule(vertBinary);
    rad::Ref<ShaderModule> frag = device->CreateShaderModule(fragBinary);
    pipelineInfo->m_shaderStages =
    {
        { VK_SHADER_STAGE_VERTEX_BIT, vert, nullptr },
        { VK_SHADER_STAGE_FRAGMENT_BIT, frag, nullptr },
    };
    pipelineInfo->m_inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInfo->SetColorBlendDisabled(1);
    pipelineInfo->m_layout = m_blitPipelineLayout;
    pipelineInfo->SetRenderingInfo(m_surfaceFormat.format);
    m_blitPipeline = device->CreateGraphicsPipeline(pipelineInfo->Setup());

    // Resource bingings:
    // set0, binding0: combined image sampler: renderTarget;
    // set0, binding1: combined image sampler: overlay;
    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 },
    };
    m_blitDescPool = device->CreateDescriptorPool(1, poolSizes);
    m_blitDescSet = m_blitDescPool->Allocate(m_blitDescSetLayout.get());

    return true;
}

void Window::UpdateBlitBindings()
{
    if (m_blitDescSet)
    {
        VkWriteDescriptorSet writes[2] = {};

        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].pNext = nullptr;
        writes[0].dstSet = m_blitDescSet->GetHandle();
        writes[0].dstBinding = 0;
        writes[0].dstArrayElement = 0;
        writes[0].descriptorCount = 1;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        VkDescriptorImageInfo renderTargetInfo = {};
        if (m_renderTarget->GetWidth() <= m_swapchain->GetWidth() &&
            m_renderTarget->GetHeight() <= m_swapchain->GetHeight())
        {
            renderTargetInfo.sampler = m_samplerNearest->GetHandle();
        }
        else
        {
            renderTargetInfo.sampler = m_samplerLinear->GetHandle();
        }
        renderTargetInfo.imageView = m_renderTargetView->GetHandle();
        renderTargetInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        writes[0].pImageInfo = &renderTargetInfo;

        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].pNext = nullptr;
        writes[1].dstSet = m_blitDescSet->GetHandle();
        writes[1].dstBinding = 1;
        writes[1].dstArrayElement = 0;
        writes[1].descriptorCount = 1;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        VkDescriptorImageInfo overlayInfo = {};
        renderTargetInfo.sampler = m_samplerNearest->GetHandle();
        renderTargetInfo.imageView = m_overlayView->GetHandle();
        renderTargetInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        writes[1].pImageInfo = &renderTargetInfo;

        m_blitDescSet->Update(writes);
    }
}

void Window::SetRenderTarget(rad::Ref<Image> renderTarget, rad::Ref<ImageView> renderTargetView)
{
    m_renderTarget = renderTarget;
    if (!renderTargetView)
    {
        m_renderTargetView = renderTarget->CreateDefaultView();
    }
}

void Window::BlitToSwapchain()
{
    Device* device = m_context->GetDevice();
    Queue* queue = m_context->GetQueue();

    CommandBuffer* cmdBuffer = m_cmdBuffers[m_cmdBufferIndex].get();

    cmdBuffer->Begin();

    if (m_renderTarget->GetCurrentLayout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        cmdBuffer->TransitLayoutFromCurrent(m_renderTarget.get(),
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    if (m_overlay->GetCurrentLayout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        cmdBuffer->TransitLayoutFromCurrent(m_overlay.get(),
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    Image* renderTarget = m_swapchain->GetCurrentImage();
    if (renderTarget->GetCurrentLayout() != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        cmdBuffer->TransitLayoutFromCurrent(renderTarget,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    ImageView* renderTargetView = m_swapchain->GetCurrentImageView();
    VkClearColorValue clearColor = {};
    cmdBuffer->BeginRendering(renderTargetView, &clearColor);
    cmdBuffer->BindPipeline(m_blitPipeline.get());
    cmdBuffer->BindDescriptorSets(
        m_blitPipeline.get(), m_blitPipelineLayout.get(),
        0, m_blitDescSet.get());

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = float(m_swapchain->GetWidth());
    viewport.height = float(m_swapchain->GetHeight());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmdBuffer->SetViewports(viewport);

    VkRect2D scissor = {};
    scissor.extent.width = m_swapchain->GetWidth();
    scissor.extent.height = m_swapchain->GetHeight();
    cmdBuffer->SetScissors(scissor);

    cmdBuffer->Draw(3, 1, 0, 0);

    cmdBuffer->EndRendering();

    if (renderTarget->GetCurrentLayout() != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        cmdBuffer->TransitLayoutFromCurrent(renderTarget,
            VK_PIPELINE_STAGE_NONE,
            VK_ACCESS_NONE,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }
    cmdBuffer->End();

    queue->Submit(
        std::array{
            cmdBuffer
        },
        std::array{ // wait
            SubmitWaitInfo {
                .semaphore = m_swapchainImageAcquired[m_backBufferIndex].get(),
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
            }
        },
        std::array{ // signal
            m_drawComplete[m_backBufferIndex].get()
        },
        m_fences[m_backBufferIndex].get()
    );
}

void Window::Present()
{
    Device* device = m_context->GetDevice();
    Queue* queue = m_context->GetQueue();

    VkResult err = queue->Present(
        std::array{ // wait
            m_drawComplete[m_backBufferIndex].get()
        },
        std::array{
            m_swapchain.get()
        }
    );

    if (err != VK_SUCCESS)
    {
        if (err == VK_SUBOPTIMAL_KHR)
        {
            m_surfaceCaps = device->GetPhysicalDevice()->
                GetSurfaceCapabilities(m_surface->GetHandle());
            int width = 0;
            int height = 0;
            GetSizeInPixels(&width, &height);
            if ((m_surfaceCaps.currentExtent.width != uint32_t(width)) ||
                (m_surfaceCaps.currentExtent.height != uint32_t(height)))
            {
                Resize(width, height);
            }
        }
        else if (err == VK_ERROR_OUT_OF_DATE_KHR)
        {
            Resize();
        }
        else if (err == VK_ERROR_SURFACE_LOST_KHR)
        {
            if (m_surface = CreateSurface())
            {
                Resize();
            }
        }
        else if (err < 0)
        {
            VKPP_LOG(err, "queue->Present failed with {}.",
                string_VkResult(err));
            throw Error(err);
        }
    }
}

} // namespace vkpp
