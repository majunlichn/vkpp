#include "Context.h"

namespace vkpp
{

Context::Context()
{
}

Context::~Context()
{
}

bool Context::Init(rad::Ref<Instance> instance, rad::Ref<PhysicalDevice> gpuSelected)
{
    m_instance = std::move(instance);

    if (!gpuSelected)
    {
        auto gpus = m_instance->EnumeratePhysicalDevices();
        for (size_t i = 0; i < gpus.size(); ++i)
        {
            VKPP_LOG(info, "Vulkan device#{}: {} (0x{:04X})", i,
                gpus[i]->GetDeviceName(), gpus[i]->GetDeviceID());
        }

        if (gpus.empty())
        {
            VKPP_LOG(err, "No Vulkan device available!");
            return false;
        }

        // Prefer the first discrete GPU.
        for (auto& gpu : gpus)
        {
            const VkPhysicalDeviceProperties& properties = gpu->m_properties;
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                gpuSelected = gpu;
                break;
            }
        }

        if (!gpuSelected)
        {
            gpuSelected = gpus[0];
        }
    }

    uint32_t apiVersion = gpuSelected->m_properties.apiVersion;
    VKPP_LOG(info, "Device selected: {} (version={}.{}.{})",
        gpuSelected->GetDeviceName(),
        VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion));

    std::set<std::string> extensionNames;
    for (const VkExtensionProperties& extension : gpuSelected->m_extensions)
    {
        if (std::string_view(extension.extensionName).starts_with("VK_KHR") ||
            std::string_view(extension.extensionName).starts_with("VK_EXT"))
        {
            extensionNames.insert(extension.extensionName);
        }
    }

    if (!m_instance->IsExtensionSupported("VK_KHR_get_physical_device_properties2") ||
        !m_instance->IsExtensionSupported("VK_KHR_surface") ||
        !m_instance->IsExtensionSupported("VK_KHR_get_surface_capabilities2") ||
        !m_instance->IsExtensionSupported("VK_KHR_swapchain"))
    {
        extensionNames.erase("VK_EXT_full_screen_exclusive");
    }

    if (extensionNames.find("VK_EXT_surface_maintenance1") == extensionNames.end())
    {
        extensionNames.erase("VK_EXT_swapchain_maintenance1");
    }

    if (extensionNames.find("VK_KHR_buffer_device_address") != extensionNames.end())
    {
        extensionNames.erase("VK_EXT_buffer_device_address");
    }

    m_device = gpuSelected->CreateDevice(extensionNames);

    for (uint32_t i = 0; i < QueueFamilyCount; ++i)
    {
        QueueFamily queueFamily = QueueFamily(i);
        if (m_device->IsQueueFamilySupported(queueFamily))
        {
            m_queues[i] = m_device->CreateQueue(QueueFamily(queueFamily));
            m_cmdPools[i] = m_device->CreateCommandPool(
                queueFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
        }
    }

    return true;
}

rad::Ref<CommandBuffer> Context::AllocateTransientCommandBuffer(
    QueueFamily queueFamily, VkCommandBufferLevel level)
{
    std::lock_guard<std::mutex> lock(m_cmdPoolMutex);
    return m_cmdPools[queueFamily]->Allocate(level);
}

void Context::WaitIdle()
{
    m_device->WaitIdle();
}

void Context::ReadBuffer(Buffer* buffer, void* dest, VkDeviceSize offset, VkDeviceSize size)
{
    if (buffer->IsHostVisible())
    {
        buffer->Read(dest, offset, size);
    }
    else
    {
        rad::Ref<Buffer> stagingBuffer = m_device->CreateStagingBuffer(size);

        QueueFamily queueFamily = QueueFamilyUniversal;
        rad::Ref<CommandBuffer> cmdBuffer = AllocateTransientCommandBuffer(queueFamily);

        cmdBuffer->Begin();

        VkBufferMemoryBarrier srcBarrier = {};
        srcBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        srcBarrier.pNext;
        srcBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        srcBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        srcBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcBarrier.buffer = buffer->GetHandle();
        srcBarrier.offset = offset;
        srcBarrier.size = size;
        cmdBuffer->SetPipelineBarrier(
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            {}, srcBarrier, {});

        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = offset;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        cmdBuffer->CopyBuffer(buffer, stagingBuffer.get(), copyRegion);

        VkBufferMemoryBarrier hostReadBarrier = {};
        hostReadBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        hostReadBarrier.pNext;
        hostReadBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        hostReadBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
        hostReadBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        hostReadBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        hostReadBarrier.buffer = stagingBuffer->GetHandle();
        hostReadBarrier.offset = 0;
        hostReadBarrier.size = size;
        cmdBuffer->SetPipelineBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_HOST_BIT,
            0,
            {}, hostReadBarrier, {});

        cmdBuffer->End();

        GetQueue(queueFamily)->SubmitAndWait(cmdBuffer.get());
        stagingBuffer->Read(dest);
    }
}

void Context::ReadBuffer(Buffer* buffer, void* dest)
{
    ReadBuffer(buffer, dest, 0, buffer->GetSize());
}

void Context::WriteBuffer(
    Buffer* buffer, const void* data, VkDeviceSize offset, VkDeviceSize size)
{
    if (buffer->IsHostVisible())
    {
        buffer->Write(data, offset, size);
    }
    else
    {
        QueueFamily queueFamily = QueueFamilyUniversal;
        rad::Ref<Buffer> stagingBuffer = m_device->CreateStagingBuffer(size);
        stagingBuffer->Write(data);

        rad::Ref<CommandBuffer> cmdBuffer = AllocateTransientCommandBuffer(queueFamily);
        cmdBuffer->Begin();
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = offset;
        copyRegion.size = size;
        cmdBuffer->CopyBuffer(stagingBuffer.get(), buffer, copyRegion);
        cmdBuffer->End();
        GetQueue(queueFamily)->SubmitAndWait(cmdBuffer.get());
    }
}

void Context::WriteBuffer(Buffer* buffer, const void* data)
{
    WriteBuffer(buffer, data, 0, buffer->GetSize());
}

void Context::CopyBufferToImage(Buffer* buffer, Image* image, rad::Span<VkBufferImageCopy> copyInfos)
{
    rad::Ref<CommandBuffer> commandBuffer = AllocateTransientCommandBuffer();
    commandBuffer->Begin();
    // VUID-vkCmdCopyBufferToImage-dstImageLayout-01396
    if (image->GetCurrentLayout() != VK_IMAGE_LAYOUT_GENERAL &&
        image->GetCurrentLayout() != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        image->GetCurrentLayout() != VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR)
    {
        commandBuffer->TransitLayoutFromCurrent(image,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    }
    commandBuffer->CopyBufferToImage(buffer, image, image->GetCurrentLayout(), copyInfos);
    commandBuffer->TransitLayoutFromCurrent(image,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    commandBuffer->End();
    GetQueue()->SubmitAndWait(commandBuffer.get());
}

void Context::CopyBufferToImage2D(
    Buffer* buffer, VkDeviceSize bufferOffset,
    Image* image, uint32_t baseMipLevel, uint32_t levelCount,
    uint32_t baseArrayLayer, uint32_t layerCount)
{
    std::vector<VkBufferImageCopy> copyInfos(levelCount);
    VkExtent3D blockExtent = vkuFormatTexelBlockExtent(image->GetFormat());
    uint32_t blockSize = vkuFormatElementSize(image->GetFormat());
    for (uint32_t i = 0; i < levelCount; i++)
    {
        uint32_t mipLevel = baseMipLevel + i;
        uint32_t mipWidth = std::max<uint32_t>(image->GetWidth() >> mipLevel, 1);
        uint32_t mipHeight = std::max<uint32_t>(image->GetHeight() >> mipLevel, 1);

        copyInfos[i].bufferOffset = bufferOffset;
        copyInfos[i].bufferRowLength = rad::RoundUpToMultiple(mipWidth, blockExtent.width);
        copyInfos[i].bufferImageHeight = rad::RoundUpToMultiple(mipHeight, blockExtent.height);
        copyInfos[i].imageSubresource.aspectMask = GetImageAspectFromFormat(image->GetFormat());
        copyInfos[i].imageSubresource.mipLevel = mipLevel;
        copyInfos[i].imageSubresource.baseArrayLayer = baseArrayLayer;
        copyInfos[i].imageSubresource.layerCount = layerCount;
        copyInfos[i].imageOffset = {};
        copyInfos[i].imageExtent.width = mipWidth;
        copyInfos[i].imageExtent.height = mipHeight;
        copyInfos[i].imageExtent.depth = 1;

        bufferOffset += (copyInfos[i].bufferRowLength / blockExtent.width) *
            (copyInfos[i].bufferImageHeight / blockExtent.height) * blockSize * layerCount;
    }
    CopyBufferToImage(buffer, image, copyInfos);
}

} // namespace vkpp
