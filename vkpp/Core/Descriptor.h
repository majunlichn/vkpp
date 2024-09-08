#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class DescriptorPool : public rad::RefCounted<DescriptorPool>
{
public:
    DescriptorPool(
        rad::Ref<Device> device,
        const VkDescriptorPoolCreateInfo& createInfo);
    ~DescriptorPool();
    VKPP_DISABLE_COPY_AND_MOVE(DescriptorPool);

    VkDescriptorPool GetHandle() const { return m_handle; }

    rad::Ref<DescriptorSet> Allocate(DescriptorSetLayout* layout);
    void Reset();

private:
    rad::Ref<Device>        m_device;
    VkDescriptorPool        m_handle = VK_NULL_HANDLE;

}; // class DescriptorPool

class DescriptorSetLayout : public rad::RefCounted<DescriptorSetLayout>
{
public:
    DescriptorSetLayout(
        rad::Ref<Device> device,
        const VkDescriptorSetLayoutCreateInfo& createInfo);
    ~DescriptorSetLayout();
    VKPP_DISABLE_COPY_AND_MOVE(DescriptorSetLayout);

    VkDescriptorSetLayout GetHandle() const { return m_handle; }

private:
    rad::Ref<Device>        m_device;
    VkDescriptorSetLayout   m_handle = VK_NULL_HANDLE;

}; // class DescriptorSetLayout

class DescriptorSet : public rad::RefCounted<DescriptorSet>
{
public:
    DescriptorSet(
        rad::Ref<Device> device,
        rad::Ref<DescriptorPool> descriptorPool,
        rad::Ref<DescriptorSetLayout> layout);
    ~DescriptorSet();
    VKPP_DISABLE_COPY_AND_MOVE(DescriptorSet);

    VkDescriptorSet GetHandle() const { return m_handle; }

    void Update(rad::Span<VkWriteDescriptorSet> writes,
        rad::Span<VkCopyDescriptorSet> copies = {});
    void UpdateBuffers(
        uint32_t binding,
        uint32_t arrayElement,
        VkDescriptorType type,
        rad::Span<VkDescriptorBufferInfo> bufferInfos);
    void UpdateUniformBuffers(
        uint32_t binding,
        uint32_t arrayElement,
        rad::Span<VkDescriptorBufferInfo> bufferInfos);
    void UpdateImages(
        uint32_t binding,
        uint32_t arrayElement,
        VkDescriptorType type,
        rad::Span<ImageView*> imageViews,
        rad::Span<VkImageLayout> layouts);
    void UpdateCombinedImageSamplers(
        uint32_t binding,
        uint32_t arrayElement,
        rad::Span<ImageView*> imageViews,
        rad::Span<VkImageLayout> layouts,
        rad::Span<Sampler*> samplers);

private:
    rad::Ref<Device>                m_device;
    rad::Ref<DescriptorPool>        m_descriptorPool;
    rad::Ref<DescriptorSetLayout>   m_layout;
    VkDescriptorSet                 m_handle = VK_NULL_HANDLE;

}; // class DescriptorSet

} // namespace vkpp
