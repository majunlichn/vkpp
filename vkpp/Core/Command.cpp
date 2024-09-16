#include <vkpp/Core/Command.h>
#include <vkpp/Core/Device.h>
#include <vkpp/Core/Fence.h>
#include <vkpp/Core/Semaphore.h>
#include <vkpp/Core/Event.h>
#include <vkpp/Core/RenderPass.h>
#include <vkpp/Core/Framebuffer.h>
#include <vkpp/Core/Pipeline.h>
#include <vkpp/Core/Buffer.h>
#include <vkpp/Core/Image.h>
#include <vkpp/Core/Sampler.h>
#include <vkpp/Core/Descriptor.h>
#include <vkpp/Core/Swapchain.h>

#include <rad/Container/SmallVector.h>

namespace vkpp
{

CommandPool::CommandPool(
    rad::Ref<Device> device,
    const VkCommandPoolCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateCommandPool(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

CommandPool::~CommandPool()
{
    m_device->GetFunctionTable()->
        vkDestroyCommandPool(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}

rad::Ref<CommandBuffer> CommandPool::Allocate(VkCommandBufferLevel level)
{
    return RAD_NEW CommandBuffer(m_device, this, level);
}

void CommandPool::Trim()
{
    m_device->GetFunctionTable()->
        vkTrimCommandPool(m_device->GetHandle(), m_handle, 0);
}

void CommandPool::Reset(VkCommandPoolResetFlags flags)
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkResetCommandPool(m_device->GetHandle(), m_handle, flags));
}

CommandBuffer::CommandBuffer(
    rad::Ref<Device> device,
    rad::Ref<CommandPool> commandPool,
    VkCommandBufferLevel level) :
    m_device(std::move(device)),
    m_commandPool(std::move(commandPool)),
    m_level(level)
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.commandPool = m_commandPool->GetHandle();
    allocateInfo.level = level;
    allocateInfo.commandBufferCount = 1;
    VK_CHECK(m_device->GetFunctionTable()->
        vkAllocateCommandBuffers(m_device->GetHandle(), &allocateInfo, &m_handle));
}

CommandBuffer::~CommandBuffer()
{
    m_device->GetFunctionTable()->
        vkFreeCommandBuffers(m_device->GetHandle(), m_commandPool->GetHandle(), 1, &m_handle);
}

void CommandBuffer::Begin(
    VkCommandBufferUsageFlags flags,
    const VkCommandBufferInheritanceInfo* pInheritanceInfo)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = flags;
    beginInfo.pInheritanceInfo = pInheritanceInfo;

    VK_CHECK(m_device->GetFunctionTable()->
        vkBeginCommandBuffer(m_handle, &beginInfo));
}

void CommandBuffer::End()
{
    VK_CHECK(m_device->GetFunctionTable()->vkEndCommandBuffer(m_handle));
}

void CommandBuffer::Reset(VkCommandBufferResetFlags flags)
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkResetCommandBuffer(m_handle, flags));
}

void CommandBuffer::BeginRenderPass(
    const VkRenderPassBeginInfo& beginInfo,
    VkSubpassContents contents)
{
    m_device->GetFunctionTable()->
        vkCmdBeginRenderPass(m_handle, &beginInfo, contents);
}

void CommandBuffer::BeginRenderPass(
    RenderPass* renderPass, Framebuffer* framebuffer,
    const VkRect2D& renderArea, rad::Span<VkClearValue> clearValues,
    VkSubpassContents contents)
{
    VkRenderPassBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.renderPass = renderPass->GetHandle();
    beginInfo.framebuffer = framebuffer->GetHandle();
    beginInfo.renderArea = renderArea;
    beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    beginInfo.pClearValues = clearValues.data();

    BeginRenderPass(beginInfo, contents);
}

void CommandBuffer::BeginRenderPass(
    RenderPass* renderPass,
    Framebuffer* framebuffer,
    rad::Span<VkClearValue> clearValues)
{
    VkRenderPassBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.renderPass = renderPass->GetHandle();
    beginInfo.framebuffer = framebuffer->GetHandle();
    beginInfo.renderArea.offset.x = 0;
    beginInfo.renderArea.offset.y = 0;
    beginInfo.renderArea.extent.width = framebuffer->GetWidth();
    beginInfo.renderArea.extent.height = framebuffer->GetHeight();
    beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    beginInfo.pClearValues = clearValues.data();

    BeginRenderPass(beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::EndRenderPass()
{
    m_device->GetFunctionTable()->vkCmdEndRenderPass(m_handle);
}

void CommandBuffer::NextSubpass(VkSubpassContents contents)
{
    m_device->GetFunctionTable()->
        vkCmdNextSubpass(m_handle, contents);
}


void CommandBuffer::BeginRendering(const VkRenderingInfoKHR& renderingInfo)
{
    vkCmdBeginRenderingKHR(m_handle, &renderingInfo);
}

void CommandBuffer::BeginRendering(
    rad::Span<ImageView*> colorViews,
    const VkClearColorValue* clearColor,
    ImageView* depthStencilView,
    const VkClearDepthStencilValue* clearDepthStencil)
{
    std::vector<VkRenderingAttachmentInfoKHR> colorInfos = {};
    colorInfos.reserve(colorViews.size());
    for (auto colorView : colorViews)
    {
        VkRenderingAttachmentInfoKHR colorInfo = {};
        colorInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        colorInfo.imageView = colorView->GetHandle();
        colorInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        if (clearColor)
        {
            colorInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorInfo.clearValue.color = *clearColor;
        }
        else
        {
            colorInfo.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
        colorInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorInfos.push_back(colorInfo);
    }

    VkRenderingAttachmentInfoKHR depthStencilInfo = {};
    if (depthStencilView)
    {
        depthStencilInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        depthStencilInfo.imageView = depthStencilView->GetHandle();
        depthStencilInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        if (clearDepthStencil)
        {
            depthStencilInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthStencilInfo.clearValue.depthStencil = *clearDepthStencil;
        }
        else
        {
            depthStencilInfo.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
        depthStencilInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    if (colorViews.size() > 0)
    {
        width = colorViews[0]->GetImage()->GetWidth();
        height = colorViews[0]->GetImage()->GetHeight();
    }
    else if (depthStencilView)
    {
        width = depthStencilView->GetImage()->GetWidth();
        height = depthStencilView->GetImage()->GetHeight();
    }

    VkRenderingInfoKHR renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderingInfo.renderArea = { 0, 0, width, height };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = colorInfos.data();
    if (depthStencilView)
    {
        renderingInfo.pDepthAttachment = &depthStencilInfo;
        renderingInfo.pStencilAttachment = &depthStencilInfo;
    }

    BeginRendering(renderingInfo);
}

void CommandBuffer::EndRendering()
{
    vkCmdEndRenderingKHR(m_handle);
}

void CommandBuffer::BindPipeline(Pipeline* pipeline)
{
    m_device->GetFunctionTable()->
        vkCmdBindPipeline(m_handle, pipeline->GetBindPoint(), pipeline->GetHandle());
}

void CommandBuffer::BindDescriptorSets(
    Pipeline* pipeline,
    PipelineLayout* layout,
    uint32_t                    firstSet,
    rad::Span<DescriptorSet*>   descSets,
    rad::Span<uint32_t>         dynamicOffsets)
{
    rad::SmallVector<VkDescriptorSet, 8> descSetsHandles(descSets.size());
    for (size_t i = 0; i < descSets.size(); i++)
    {
        descSetsHandles[i] = descSets[i]->GetHandle();
    }

    m_device->GetFunctionTable()->
        vkCmdBindDescriptorSets(m_handle, pipeline->GetBindPoint(), layout->GetHandle(),
            firstSet, static_cast<uint32_t>(descSetsHandles.size()), descSetsHandles.data(),
            static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
}

void CommandBuffer::SetScissors(rad::Span<VkRect2D> scissors, uint32_t first)
{
    m_device->GetFunctionTable()->
        vkCmdSetScissor(m_handle, first, static_cast<uint32_t>(scissors.size()), scissors.data());
}

void CommandBuffer::SetViewports(rad::Span<VkViewport> viewports, uint32_t first)
{
    m_device->GetFunctionTable()->
        vkCmdSetViewport(m_handle, first, static_cast<uint32_t>(viewports.size()), viewports.data());
}

void CommandBuffer::SetDepthBounds(float min, float max)
{
    m_device->GetFunctionTable()->
        vkCmdSetDepthBounds(m_handle, min, max);
}

void CommandBuffer::SetDepthStencilCompareMask(
    VkStencilFaceFlags faceMask, uint32_t compareMask)
{
    m_device->GetFunctionTable()->
        vkCmdSetStencilCompareMask(m_handle, faceMask, compareMask);
}

void CommandBuffer::SetStencilWriteMask(
    VkStencilFaceFlags faceMask, uint32_t writeMask)
{
    m_device->GetFunctionTable()->
        vkCmdSetStencilWriteMask(m_handle, faceMask, writeMask);
}

void CommandBuffer::SetStencilReference(
    VkStencilFaceFlags faceMask, uint32_t reference)
{
    m_device->GetFunctionTable()->
        vkCmdSetStencilReference(m_handle, faceMask, reference);
}

void CommandBuffer::SetBlendConstants(float r, float g, float b, float a)
{
    float blendConstants[4] = { r, g, b, a };
    m_device->GetFunctionTable()->
        vkCmdSetBlendConstants(m_handle, blendConstants);
}

void CommandBuffer::SetLineWidth(float width)
{
    m_device->GetFunctionTable()->
        vkCmdSetLineWidth(m_handle, width);
}

void CommandBuffer::SetDepthBias(
    float constantFactor, float clamp, float slopeFactor)
{
    m_device->GetFunctionTable()->
        vkCmdSetDepthBias(m_handle, constantFactor, clamp, slopeFactor);
}

void CommandBuffer::BindIndexBuffer(
    Buffer* buffer, VkDeviceSize offset, VkIndexType indexType)
{
    m_device->GetFunctionTable()->
        vkCmdBindIndexBuffer(m_handle, buffer->GetHandle(), offset, indexType);
}

void CommandBuffer::BindVertexBuffers(
    uint32_t firstBinding, rad::Span<Buffer*> buffers, rad::Span<VkDeviceSize> offsets)
{
    rad::SmallVector<VkBuffer, 8> buffersHandles(buffers.size());
    for (size_t i = 0; i < buffersHandles.size(); i++)
    {
        buffersHandles[i] = buffers[i]->GetHandle();
    }

    m_device->GetFunctionTable()->vkCmdBindVertexBuffers(m_handle,
        firstBinding, static_cast<uint32_t>(buffersHandles.size()), buffersHandles.data(), offsets.data());
}

void CommandBuffer::BindVertexBuffers(
    uint32_t firstBinding, rad::Span<Buffer*> buffers)
{
    rad::SmallVector<VkDeviceSize, 8> offsets(buffers.size(), 0);
    return BindVertexBuffers(firstBinding, buffers, { offsets.data(), offsets.size() });
}

void CommandBuffer::Draw(
    uint32_t vertexCount, uint32_t instanceCount,
    uint32_t firstVertex, uint32_t firstInstance)
{
    m_device->GetFunctionTable()->vkCmdDraw(
        m_handle, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::DrawIndexed(
    uint32_t indexCount, uint32_t instanceCount,
    uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    m_device->GetFunctionTable()->
        vkCmdDrawIndexed(m_handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::DrawIndirect(
    Buffer* buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
    m_device->GetFunctionTable()->
        vkCmdDrawIndirect(m_handle, buffer->GetHandle(), offset, drawCount, stride);
}

void CommandBuffer::DrawIndexedIndirect(
    Buffer* buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
    m_device->GetFunctionTable()->
        vkCmdDrawIndexedIndirect(m_handle, buffer->GetHandle(), offset, drawCount, stride);
}

void CommandBuffer::Dispatch(
    uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    m_device->GetFunctionTable()->
        vkCmdDispatch(m_handle, groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::DispatchIndirect(Buffer* buffer, VkDeviceSize offset)
{
    m_device->GetFunctionTable()->vkCmdDispatchIndirect(m_handle, buffer->GetHandle(), offset);
}

void CommandBuffer::DispatchBase(
    uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
    uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    m_device->GetFunctionTable()->vkCmdDispatchBase(m_handle,
        baseGroupX, baseGroupY, baseGroupZ,
        groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::ClearColorImage(
    Image* image,
    VkImageLayout layout,
    const VkClearColorValue& color,
    rad::Span<VkImageSubresourceRange> ranges)
{
    m_device->GetFunctionTable()->
        vkCmdClearColorImage(m_handle, image->GetHandle(), layout, &color,
            static_cast<uint32_t>(ranges.size()), ranges.data());
}

void CommandBuffer::ClearDepthStencilImage(
    Image* image,
    VkImageLayout layout,
    const VkClearDepthStencilValue& value,
    rad::Span<VkImageSubresourceRange> ranges)
{
    m_device->GetFunctionTable()->
        vkCmdClearDepthStencilImage(
            m_handle, image->GetHandle(), layout, &value,
            static_cast<uint32_t>(ranges.size()), ranges.data());
}

void CommandBuffer::ClearAttachments(
    rad::Span<VkClearAttachment> attachments,
    rad::Span<VkClearRect> rects)
{
    m_device->GetFunctionTable()->
        vkCmdClearAttachments(m_handle,
            static_cast<uint32_t>(attachments.size()), attachments.data(),
            static_cast<uint32_t>(rects.size()), rects.data());
}

void CommandBuffer::FillBuffer(
    Buffer* dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data)
{
    m_device->GetFunctionTable()->
        vkCmdFillBuffer(m_handle, dstBuffer->GetHandle(), dstOffset, size, data);
}

void CommandBuffer::UpdateBuffer(Buffer* dstBuffer,
    VkDeviceSize offset, VkDeviceSize dataSize, const uint32_t* pData)
{
    m_device->GetFunctionTable()->
        vkCmdUpdateBuffer(m_handle, dstBuffer->GetHandle(), offset, dataSize, pData);
}

void CommandBuffer::CopyBuffer(
    Buffer* srcBuffer, Buffer* dstBuffer, rad::Span<VkBufferCopy> regions)
{
    m_device->GetFunctionTable()->
        vkCmdCopyBuffer(m_handle, srcBuffer->GetHandle(), dstBuffer->GetHandle(),
            static_cast<uint32_t>(regions.size()), regions.data());
}

void CommandBuffer::CopyImage(
    Image* srcImage, VkImageLayout srcImageLayout,
    Image* dstImage, VkImageLayout dstImageLayout,
    rad::Span<VkImageCopy> regions)
{
    m_device->GetFunctionTable()->
        vkCmdCopyImage(
            m_handle, srcImage->GetHandle(), srcImageLayout,
            dstImage->GetHandle(), dstImageLayout,
            static_cast<uint32_t>(regions.size()), regions.data()
        );
}

void CommandBuffer::CopyBufferToImage(
    Buffer* srcBuffer,
    Image* dstImage, VkImageLayout dstImageLayout,
    rad::Span<VkBufferImageCopy> regions)
{
    m_device->GetFunctionTable()->
        vkCmdCopyBufferToImage(m_handle, srcBuffer->GetHandle(), dstImage->GetHandle(), dstImageLayout,
            static_cast<uint32_t>(regions.size()), regions.data());
}

void CommandBuffer::CopyImageToBuffer(
    Image* srcImage, VkImageLayout srcImageLayout, Buffer* dstBuffer,
    rad::Span<VkBufferImageCopy> regions)
{
    m_device->GetFunctionTable()->
        vkCmdCopyImageToBuffer(m_handle, srcImage->GetHandle(), srcImageLayout, dstBuffer->GetHandle(),
            static_cast<uint32_t>(regions.size()), regions.data());
}

void CommandBuffer::BlitImage(
    Image* srcImage, VkImageLayout srcImageLayout,
    Image* dstImage, VkImageLayout dstImageLayout,
    rad::Span<VkImageBlit> regions, VkFilter filter)
{
    m_device->GetFunctionTable()->
        vkCmdBlitImage(m_handle, srcImage->GetHandle(), srcImageLayout, dstImage->GetHandle(), dstImageLayout,
            static_cast<uint32_t>(regions.size()), regions.data(), filter);
}

void CommandBuffer::ResolveImage(
    Image* srcImage, VkImageLayout srcImageLayout,
    Image* dstImage, VkImageLayout dstImageLayout,
    rad::Span<VkImageResolve> regions)
{
    m_device->GetFunctionTable()->vkCmdResolveImage(
        m_handle,
        srcImage->GetHandle(), srcImageLayout,
        dstImage->GetHandle(), dstImageLayout,
        static_cast<uint32_t>(regions.size()), regions.data());
}

void CommandBuffer::SetPushConstants(
    PipelineLayout* layout, VkShaderStageFlags stageFlags,
    uint32_t offset, uint32_t size, const void* pValues)
{
    m_device->GetFunctionTable()->
        vkCmdPushConstants(m_handle, layout->GetHandle(), stageFlags,
            offset, size, pValues);
}

void CommandBuffer::SetPipelineBarrier(
    VkPipelineStageFlags                srcStageMask,
    VkPipelineStageFlags                dstStageMask,
    VkDependencyFlags                   dependencyFlags,
    rad::Span<VkMemoryBarrier>          memoryBarriers,
    rad::Span<VkBufferMemoryBarrier>    bufferMemoryBarriers,
    rad::Span<VkImageMemoryBarrier>     imageMemoryBarriers)
{
    m_device->GetFunctionTable()->vkCmdPipelineBarrier(
        m_handle, srcStageMask, dstStageMask, dependencyFlags,
        static_cast<uint32_t>(memoryBarriers.size()), memoryBarriers.data(),
        static_cast<uint32_t>(bufferMemoryBarriers.size()), bufferMemoryBarriers.data(),
        static_cast<uint32_t>(imageMemoryBarriers.size()), imageMemoryBarriers.data()
    );
}

void CommandBuffer::SetPipelineBarrier2(
    VkDependencyFlags                   dependencyFlags,
    rad::Span<VkMemoryBarrier2>         memoryBarriers,
    rad::Span<VkBufferMemoryBarrier2>   bufferMemoryBarriers,
    rad::Span<VkImageMemoryBarrier2>    imageMemoryBarriers)
{
    VkDependencyInfo dependencyInfo = {};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.pNext = nullptr;
    dependencyInfo.dependencyFlags = dependencyFlags;
    dependencyInfo.memoryBarrierCount = static_cast<uint32_t>(memoryBarriers.size());
    dependencyInfo.pMemoryBarriers = memoryBarriers.data();
    dependencyInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(bufferMemoryBarriers.size());
    dependencyInfo.pBufferMemoryBarriers = bufferMemoryBarriers.data();
    dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(imageMemoryBarriers.size());
    dependencyInfo.pImageMemoryBarriers = imageMemoryBarriers.data();
    m_device->GetFunctionTable()->vkCmdPipelineBarrier2(m_handle, &dependencyInfo);
}

void CommandBuffer::SetPipelineBarrier2(const VkDependencyInfo& dependencyInfo)
{
    m_device->GetFunctionTable()->vkCmdPipelineBarrier2KHR(m_handle, &dependencyInfo);
}

void CommandBuffer::SetMemoryBarrier2(
    VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask,
    VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask)
{
    VkMemoryBarrier2 memoryBarrier = {};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.pNext = nullptr;
    memoryBarrier.srcAccessMask = srcAccessMask;
    memoryBarrier.dstAccessMask = dstAccessMask;
    this->SetPipelineBarrier2(0, memoryBarrier, {}, {});
}

void CommandBuffer::SetMemoryBarrier_ComputeToCompute_ReadAfterWrite2()
{
    this->SetMemoryBarrier2(
        VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR, VK_ACCESS_2_SHADER_WRITE_BIT_KHR,
        VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR, VK_ACCESS_2_SHADER_READ_BIT_KHR);
}

void CommandBuffer::SetExecutionDependency2(
    VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask)
{
    VkMemoryBarrier2 memoryBarrier = {};
    memoryBarrier.srcStageMask = srcStageMask;
    memoryBarrier.dstStageMask = dstStageMask;
    VkDependencyInfo dependencyInfo = {};
    dependencyInfo.memoryBarrierCount = 1;
    dependencyInfo.pMemoryBarriers = &memoryBarrier;
    this->SetPipelineBarrier2(dependencyInfo);
}

void CommandBuffer::SetExecutionDependency2_ComputeToCompute()
{
    SetExecutionDependency2(VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
        VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR);
}

void CommandBuffer::TransitLayout(Image* image,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkAccessFlags srcAccessMask,
    VkAccessFlags dstAccessMask,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    const VkImageSubresourceRange* subresourceRange)
{
    VkImageMemoryBarrier imageBarrier = {};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.pNext = nullptr;
    imageBarrier.srcAccessMask = srcAccessMask;
    imageBarrier.dstAccessMask = dstAccessMask;
    imageBarrier.oldLayout = oldLayout;
    imageBarrier.newLayout = newLayout;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.image = image->GetHandle();
    if (subresourceRange)
    {
        imageBarrier.subresourceRange = *subresourceRange;
    }
    else
    {
        imageBarrier.subresourceRange.aspectMask =
            GetImageAspectFromFormat(image->GetFormat());
        imageBarrier.subresourceRange.baseMipLevel = 0;
        imageBarrier.subresourceRange.levelCount =
            image->GetMipLevels();
        imageBarrier.subresourceRange.baseArrayLayer = 0;
        imageBarrier.subresourceRange.layerCount =
            image->GetArrayLayers();
    }
    this->SetPipelineBarrier(
        srcStageMask,    // srcStageMask
        dstStageMask,    // dstStageMask
        0,
        {},
        {},
        { &imageBarrier, 1 }
    );
    image->SetCurrentPipelineStage(dstStageMask);
    image->SetCurrentAccessFlags(dstAccessMask);
    image->SetCurrentLayout(newLayout);
}

void CommandBuffer::TransitLayoutFromCurrent(
    Image* image,
    VkPipelineStageFlags    dstStageMask,
    VkAccessFlags           dstAccessMask,
    VkImageLayout           newLayout,
    const VkImageSubresourceRange* subresourceRange)
{
    TransitLayout(image,
        image->GetCurrentPipelineStage(), dstStageMask,
        image->GetCurrentAccessMask(), dstAccessMask,
        image->GetCurrentLayout(), newLayout,
        subresourceRange);
}

} // namespace vkpp
