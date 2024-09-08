#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class CommandPool : public rad::RefCounted<CommandPool>
{
public:
    CommandPool(rad::Ref<Device> device, const VkCommandPoolCreateInfo& createInfo);
    ~CommandPool();
    VKPP_DISABLE_COPY_AND_MOVE(CommandPool);

    VkCommandPool GetHandle() const { return m_handle; }

    rad::Ref<CommandBuffer> Allocate(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Recycles unused memory.
    void Trim();
    // Recycles all of the resources from all of the command buffers allocated.
    void Reset(VkCommandPoolResetFlags flags = 0);

private:
    rad::Ref<Device>        m_device;
    VkCommandPool           m_handle = VK_NULL_HANDLE;

}; // class CommandPool

class CommandBuffer : public rad::RefCounted<CommandBuffer>
{
public:
    CommandBuffer(
        rad::Ref<Device>            device,
        rad::Ref<CommandPool>       commandPool,
        VkCommandBufferLevel        level);
    ~CommandBuffer();
    VKPP_DISABLE_COPY_AND_MOVE(CommandBuffer);

    VkCommandBuffer GetHandle() const { return m_handle; }

    void CreateCheckpoint(
        VkCommandBufferUsageFlags flags = 0,
        const VkCommandBufferInheritanceInfo* pInheritanceInfo = nullptr);
    void End();
    void Reset(VkCommandBufferResetFlags flags = 0);

    void BeginRenderPass(
        const VkRenderPassBeginInfo& beginInfo,
        VkSubpassContents contents);
    void BeginRenderPass(
        RenderPass* renderPass,
        Framebuffer* framebuffer,
        const VkRect2D& renderArea,
        rad::Span<VkClearValue> clearValues,
        VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
    void BeginRenderPass(
        RenderPass* renderPass,
        Framebuffer* framebuffer,
        rad::Span<VkClearValue> clearValues);
    void EndRenderPass();
    void NextSubpass(VkSubpassContents contents);

    void BeginRendering(
        const VkRenderingInfoKHR& renderingInfo);
    void BeginRendering(
        rad::Span<ImageView*> colorViews,
        const VkClearColorValue* clearColor,
        ImageView* depthStencilView = nullptr,
        const VkClearDepthStencilValue* clearDepthStencil = nullptr);
    void EndRendering();

    void BindPipeline(Pipeline* pipeline);
    void BindDescriptorSets(
        Pipeline* pipeline,
        PipelineLayout* layout,
        uint32_t firstSet,
        rad::Span<DescriptorSet*> descSets,
        rad::Span<uint32_t> dynamicOffsets = {});

    void SetScissors(rad::Span<VkRect2D> scissors, uint32_t first = 0);
    void SetViewports(rad::Span<VkViewport> viewports, uint32_t first = 0);

    void SetDepthBounds(float min, float max);

    void SetDepthStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask);
    void SetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask);
    void SetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference);
    void SetBlendConstants(float r, float g, float b, float a);

    // Rasterization

    void SetLineWidth(float width);
    void SetDepthBias(float constantFactor, float clamp, float slopeFactor);

    void BindIndexBuffer(Buffer* buffer, VkDeviceSize offset, VkIndexType indexType);
    void BindVertexBuffers(
        uint32_t                firstBinding,
        rad::Span<Buffer*>      buffers,
        rad::Span<VkDeviceSize> offsets);
    void BindVertexBuffers(uint32_t firstBinding, rad::Span<Buffer*> buffers);

    // Draw

    void Draw(
        uint32_t vertexCount, uint32_t instanceCount,
        uint32_t firstVertex, uint32_t firstInstance);
    void DrawIndexed(
        uint32_t indexCount, uint32_t instanceCount,
        uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void DrawIndirect(
        Buffer* buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
    void DrawIndexedIndirect(
        Buffer* buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);

    // Dispatch

    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
    void DispatchIndirect(Buffer* buffer, VkDeviceSize offset);
    void DispatchBase(
        uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
        uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    // Clear

    void ClearColorImage(
        Image* image, VkImageLayout layout,
        const VkClearColorValue& color, rad::Span<VkImageSubresourceRange> ranges);
    void ClearDepthStencilImage(
        Image* image, VkImageLayout layout,
        const VkClearDepthStencilValue& value, rad::Span<VkImageSubresourceRange> ranges);
    void ClearAttachments(rad::Span<VkClearAttachment> attachments, rad::Span<VkClearRect> rects);
    void FillBuffer(Buffer* dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);
    // dataSize must be less than or equal to 65536 bytes.
    void UpdateBuffer(Buffer* dstBuffer,
        VkDeviceSize offset, VkDeviceSize dataSize, const uint32_t* pData);

    // Copy

    void CopyBuffer(Buffer* srcBuffer, Buffer* dstBuffer, rad::Span<VkBufferCopy> regions);
    void CopyImage(
        Image* srcImage, VkImageLayout srcLayout,
        Image* dstImage, VkImageLayout dstLayout,
        rad::Span<VkImageCopy> regions);
    void CopyBufferToImage(Buffer* srcBuffer,
        Image* dstImage, VkImageLayout dstImageLayout,
        rad::Span<VkBufferImageCopy> regions);
    void CopyImageToBuffer(Image* srcImage, VkImageLayout srcImageLayout,
        Buffer* dstBuffer, rad::Span<VkBufferImageCopy> regions);
    void BlitImage(
        Image* srcImage, VkImageLayout srcImageLayout,
        Image* dstImage, VkImageLayout dstImageLayout,
        rad::Span<VkImageBlit> regions, VkFilter filter);
    void ResolveImage(
        Image* srcImage, VkImageLayout srcImageLayout,
        Image* dstImage, VkImageLayout dstImageLayout,
        rad::Span<VkImageResolve> regions);

    void SetPushConstants(PipelineLayout* layout, VkShaderStageFlags stageFlags,
        uint32_t offset, uint32_t size, const void* pValues);

    // Khronos Synchronization Examples:
    // https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples

    void SetPipelineBarrier(
        VkPipelineStageFlags                srcStageMask,
        VkPipelineStageFlags                dstStageMask,
        VkDependencyFlags                   dependencyFlags,
        rad::Span<VkMemoryBarrier>          memoryBarriers,
        rad::Span<VkBufferMemoryBarrier>    bufferMemoryBarriers,
        rad::Span<VkImageMemoryBarrier>     imageMemoryBarriers
    );

    void SetPipelineBarrier2(const VkDependencyInfo& dependencyInfo);
    void SetPipelineBarrier2(
        VkDependencyFlags                   dependencyFlags,
        rad::Span<VkMemoryBarrier2>         memoryBarriers,
        rad::Span<VkBufferMemoryBarrier2>   bufferMemoryBarriers,
        rad::Span<VkImageMemoryBarrier2>    imageMemoryBarriers
    );

    // Global memory barrier covers all resources.
    void SetMemoryBarrier2(
        VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask,
        VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask);
    // For comput to compute storage buffer/image read after write.
    void SetMemoryBarrier_ComputeToCompute_ReadAfterWrite2();
    // A pipeline barrier or event without a any access flags is an execution dependency.
    void SetExecutionDependency2(
        VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask);
    void SetExecutionDependency2_ComputeToCompute();

    void TransitLayout(
        Image* image,
        VkPipelineStageFlags    srcStageMask,
        VkPipelineStageFlags    dstStageMask,
        VkAccessFlags           srcAccessMask,
        VkAccessFlags           dstAccessMask,
        VkImageLayout           oldLayout,
        VkImageLayout           newLayout,
        const VkImageSubresourceRange* subresourceRange = nullptr);

    void TransitLayoutFromCurrent(
        Image* image,
        VkPipelineStageFlags    dstStageMask,
        VkAccessFlags           dstAccessMask,
        VkImageLayout           newLayout,
        const VkImageSubresourceRange* subresourceRange = nullptr);

private:
    rad::Ref<Device>            m_device;
    rad::Ref<CommandPool>       m_commandPool;
    VkCommandBuffer             m_handle = VK_NULL_HANDLE;
    VkCommandBufferLevel        m_level;

}; // class CommandBuffer

} // namespace vkpp
