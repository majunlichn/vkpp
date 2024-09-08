#pragma once

#include <vkpp/Core/Common.h>

namespace vkpp
{

class Buffer : public rad::RefCounted<Buffer>
{
public:
    Buffer(
        rad::Ref<Device> device,
        const VkBufferCreateInfo& createInfo,
        const VmaAllocationCreateInfo& allocInfo);
    ~Buffer();
    VKPP_DISABLE_COPY_AND_MOVE(Buffer);

    VkBuffer GetHandle() const { return m_handle; }

    VkDeviceSize GetSize() const { return m_size; }
    VkBufferUsageFlags GetUsage() const { return m_usage; }
    VkMemoryPropertyFlags GetMemoryFlags() const { return m_memoryFlags; }
    bool IsHostVisible() const;
    bool IsHostCoherent() const;
    VmaAllocation GetAllocation() { return m_allocation; }
    const VmaAllocationInfo& GetAllocationInfo() const { return m_allocationInfo; }
    VkDeviceAddress GetDeviceAddress() const;

    void* GetMappedAddr();
    void* MapMemory(VkDeviceSize offset, VkDeviceSize size);
    // Map whole range.
    void* MapMemory();
    void UnmapMemory();
    void FlushAllocation(VkDeviceSize offset, VkDeviceSize size);
    void FlushAllocation();
    void InvalidateAllocation(VkDeviceSize offset, VkDeviceSize size);
    void InvalidateAllocation();

    rad::Ref<BufferView> CreateBufferView(VkFormat format,
        VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE);

    VkDescriptorBufferInfo GetDescriptorInfo(VkDeviceSize offset, VkDeviceSize range) const;
    VkDescriptorBufferInfo GetDescriptorInfo() const;

    // The memory must be host visible.
    void Read(void* dest, VkDeviceSize offset, VkDeviceSize size);
    void Read(void* dest);
    void Write(const void* data, VkDeviceSize offset, VkDeviceSize size);
    void Write(const void* data);

private:
    rad::Ref<Device>        m_device;
    VkBuffer                m_handle = VK_NULL_HANDLE;
    VkDeviceSize            m_size = 0;
    VkBufferUsageFlags      m_usage;
    VkSharingMode           m_sharingMode;
    VmaAllocation           m_allocation;
    VmaAllocationInfo       m_allocationInfo;
    VkMemoryPropertyFlags   m_memoryFlags;

}; // class Buffer

class BufferView : public rad::RefCounted<BufferView>
{
public:
    BufferView(
        rad::Ref<Device> device,
        rad::Ref<Buffer> buffer,
        const VkBufferViewCreateInfo& createInfo);
    ~BufferView();
    VKPP_DISABLE_COPY_AND_MOVE(BufferView);

    VkBufferView GetHandle() const { return m_handle; }

private:
    rad::Ref<Device>        m_device;
    rad::Ref<Buffer>        m_buffer;
    VkBufferView            m_handle = VK_NULL_HANDLE;
    VkFormat                m_format = VK_FORMAT_UNDEFINED;
    VkDeviceSize            m_offset = 0;
    VkDeviceSize            m_range = 0;

}; // class BufferView

} // namespace vkpp
