#include <vkpp/Gui/GuiRenderer.h>

namespace vkpp
{

GuiRenderer::GuiRenderer(Window* window) :
    m_window(window)
{
}

GuiRenderer::~GuiRenderer()
{
    Destroy();
}

static void CheckResult(VkResult result)
{
    if (result < 0)
    {
        VKPP_LOG(err, "vkpp::GuiRenderer: {}", string_VkResult(result));
        throw Error(result);
    }
}

bool GuiRenderer::Init(rad::Span<VkDescriptorPoolSize> descPoolSizes)
{
    Context* context = m_window->GetContext();
    Instance* instance = context->GetInstance();
    Device* device = context->GetDevice();
    Queue* queue = context->GetQueue();
    Swapchain* swapchain = m_window->GetSwapchain();
    uint32_t swapchainImageCount = swapchain->GetImageCount();

    // Setup Dear ImGui context:
    IMGUI_CHECKVERSION();
    m_gui = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    // TODO: customize the style?
    ImGuiStyle* style = &ImGui::GetStyle();
    style->GrabRounding = 4.0f;

    int width = 0;
    int height = 0;
    m_window->GetSizeInPixels(&width, &height);
    m_renderTarget = device->CreateImage2DRenderTarget(
        m_renderTargetFormat, width, height, VK_IMAGE_USAGE_SAMPLED_BIT);
    m_renderTargetView = m_renderTarget->CreateDefaultView();

    std::vector<VkDescriptorPoolSize> defaultDescPoolSizes =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 128 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4096 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 8 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 8 },
    };

    if (descPoolSizes.empty())
    {
        descPoolSizes = defaultDescPoolSizes;
    }

    m_descPool = device->CreateDescriptorPool(1024, descPoolSizes);

    ImGui_ImplVulkan_LoadFunctions(
        [](const char* functionName, void* userData) {
            Instance* instance = reinterpret_cast<Instance*>(userData);
            return vkGetInstanceProcAddr(instance->GetHandle(), functionName);
        },
        instance);

    ImGui_ImplSDL3_InitForVulkan(m_window->GetHandle());
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = instance->GetHandle();
    initInfo.PhysicalDevice = device->GetPhysicalDevice()->GetHandle();
    initInfo.Device = device->GetHandle();
    initInfo.QueueFamily = queue->GetQueueFamilyIndex();
    initInfo.Queue = queue->GetHandle();
    initInfo.DescriptorPool = m_descPool->GetHandle();
    initInfo.RenderPass = nullptr;
    initInfo.MinImageCount = swapchainImageCount;
    initInfo.ImageCount = swapchainImageCount;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    // Optional:
    initInfo.PipelineCache = nullptr;
    initInfo.Subpass = 0;
    initInfo.UseDynamicRendering = true;
    auto& renderingInfo = initInfo.PipelineRenderingCreateInfo;
    renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    renderingInfo.pNext = nullptr;
    renderingInfo.viewMask = 0;
    VkFormat format = m_renderTargetFormat;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &format;
    renderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
    initInfo.Allocator = nullptr;
    initInfo.CheckVkResultFn = CheckResult;
    initInfo.MinAllocationSize = 0;
    ImGui_ImplVulkan_Init(&initInfo);

    SDL_DisplayID displayID = SDL_GetDisplayForWindow(m_window->GetHandle());
    SDL_Rect rect = {};
    float fontSize = 16.0f * SDL_GetDisplayContentScale(displayID);
    fontSize = std::max(fontSize, 16.0f);
    fontSize = (float)rad::RoundUpToMultiple(size_t(fontSize), size_t(4));
#if defined(_WIN32)
    auto fonts = ImGui::GetIO().Fonts;
    fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", fontSize);
#endif

    m_cmdPool = device->CreateCommandPool(QueueFamilyUniversal,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    m_cmdBuffers.resize(swapchainImageCount);
    for (size_t i = 0; i < swapchainImageCount; ++i)
    {
        m_cmdBuffers[i] = m_cmdPool->Allocate();
    }

    return true;
}

void GuiRenderer::Destroy()
{
    if (m_gui)
    {
        Context* context = m_window->GetContext();
        Device* device = context->GetDevice();
        device->WaitIdle();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        m_gui = nullptr;
    }
}

bool GuiRenderer::ProcessEvent(const SDL_Event& event)
{
    return ImGui_ImplSDL3_ProcessEvent(&event);
}

void GuiRenderer::NewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void GuiRenderer::Render()
{
    ImGui::Render();

    Context* context = m_window->GetContext();
    Device* device = context->GetDevice();
    Queue* queue = context->GetQueue();
    Swapchain* swapchain = m_window->GetSwapchain();

    CommandBuffer* cmdBuffer = m_cmdBuffers[m_cmdBufferIndex].get();
    cmdBuffer->Begin();
    if (m_renderTarget->GetCurrentLayout() != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        cmdBuffer->TransitLayoutFromCurrent(m_renderTarget.get(),
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
    cmdBuffer->BeginRendering(m_renderTargetView.get(), &m_clearValue);
    ImDrawData* drawData = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(drawData, cmdBuffer->GetHandle());
    cmdBuffer->EndRendering();
    cmdBuffer->End();

    queue->Submit(cmdBuffer);

    m_cmdBufferIndex += 1;
    m_cmdBufferIndex %= swapchain->GetImageCount();
}

} // namespace vkpp
