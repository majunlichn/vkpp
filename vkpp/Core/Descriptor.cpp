#include <vkpp/Core/Descriptor.h>
#include <vkpp/Core/Device.h>
#include <vkpp/Core/Buffer.h>
#include <vkpp/Core/Image.h>
#include <vkpp/Core/Sampler.h>

namespace vkpp
{

DescriptorPool::DescriptorPool(
    rad::Ref<Device> device,
    const VkDescriptorPoolCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateDescriptorPool(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

DescriptorPool::~DescriptorPool()
{
    m_device->GetFunctionTable()->
        vkDestroyDescriptorPool(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}

rad::Ref<DescriptorSet> DescriptorPool::Allocate(DescriptorSetLayout* layout)
{
    return RAD_NEW DescriptorSet(m_device, this, layout);
}

void DescriptorPool::Reset()
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkResetDescriptorPool(m_device->GetHandle(), m_handle, 0));
}

DescriptorSetLayout::DescriptorSetLayout(
    rad::Ref<Device> device,
    const VkDescriptorSetLayoutCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateDescriptorSetLayout(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    m_device->GetFunctionTable()->
        vkDestroyDescriptorSetLayout(m_device->GetHandle(), m_handle, nullptr);
}

DescriptorSet::DescriptorSet(
    rad::Ref<Device> device,
    rad::Ref<DescriptorPool> descriptorPool,
    rad::Ref<DescriptorSetLayout> layout) :
    m_device(std::move(device)),
    m_descriptorPool(std::move(descriptorPool)),
    m_layout(std::move(layout))
{
    VkDescriptorSetLayout layoutHandle = m_layout->GetHandle();

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.descriptorPool = m_descriptorPool->GetHandle();
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &layoutHandle;

    VK_CHECK(m_device->GetFunctionTable()->
        vkAllocateDescriptorSets(m_device->GetHandle(), &allocateInfo, &m_handle));
}

DescriptorSet::~DescriptorSet()
{
    m_device->GetFunctionTable()->
        vkFreeDescriptorSets(m_device->GetHandle(), m_descriptorPool->GetHandle(), 1, &m_handle);
}

void DescriptorSet::Update(
    rad::Span<VkWriteDescriptorSet> writes,
    rad::Span<VkCopyDescriptorSet> copies)
{
    m_device->GetFunctionTable()->
        vkUpdateDescriptorSets(m_device->GetHandle(),
            static_cast<uint32_t>(writes.size()), writes.data(),
            static_cast<uint32_t>(copies.size()), copies.data());
}

void DescriptorSet::UpdateBuffers(
    uint32_t binding, uint32_t arrayElement, VkDescriptorType type,
    rad::Span<VkDescriptorBufferInfo> bufferInfos)
{
    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = m_handle;
    write.dstBinding = binding;
    write.dstArrayElement = arrayElement;
    write.descriptorCount = static_cast<uint32_t>(bufferInfos.size());
    write.descriptorType = type;
    write.pImageInfo = nullptr;
    write.pBufferInfo = bufferInfos.data();
    write.pTexelBufferView = nullptr;

    Update(write);
}

void DescriptorSet::UpdateUniformBuffers(
    uint32_t binding, uint32_t arrayElement, rad::Span<VkDescriptorBufferInfo> bufferInfos)
{
    UpdateBuffers(binding, arrayElement, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfos);
}

void DescriptorSet::UpdateImages(
    uint32_t binding, uint32_t arrayElement, VkDescriptorType type,
    rad::Span<ImageView*> imageViews, rad::Span<VkImageLayout> layouts)
{
    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = m_handle;
    write.dstBinding = binding;
    write.dstArrayElement = arrayElement;
    write.descriptorCount = static_cast<uint32_t>(imageViews.size());
    write.descriptorType = type;

    std::vector<VkDescriptorImageInfo> imageInfos(imageViews.size());
    for (size_t i = 0; i < imageViews.size(); i++)
    {
        imageInfos[i].sampler = VK_NULL_HANDLE;
        imageInfos[i].imageView = imageViews[i]->GetHandle();
        imageInfos[i].imageLayout = layouts[i];
    }

    write.pImageInfo = imageInfos.data();
    write.pBufferInfo = nullptr;
    write.pTexelBufferView = nullptr;

    Update(write);
}

void DescriptorSet::UpdateCombinedImageSamplers(
    uint32_t binding, uint32_t arrayElement,
    rad::Span<ImageView*> imageViews, rad::Span<VkImageLayout> layouts,
    rad::Span<Sampler*> samplers)
{
    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = m_handle;
    write.dstBinding = binding;
    write.dstArrayElement = arrayElement;
    write.descriptorCount = static_cast<uint32_t>(imageViews.size());
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    std::vector<VkDescriptorImageInfo> imageInfos(imageViews.size());
    for (size_t i = 0; i < imageViews.size(); i++)
    {
        imageInfos[i].sampler = samplers[i]->GetHandle();
        imageInfos[i].imageView = imageViews[i]->GetHandle();
        imageInfos[i].imageLayout = layouts[i];
    }

    write.pImageInfo = imageInfos.data();
    write.pBufferInfo = nullptr;
    write.pTexelBufferView = nullptr;

    Update(write);
}

} // namespace vkpp
