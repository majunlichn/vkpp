#pragma once

#include <vkpp/Core/Context.h>
#include <vkpp/Core/Math.h>
#include <rad/Container/SmallVector.h>
#include <rad/Container/Span.h>

namespace vkpp
{

class Tensor : public rad::RefCounted<Tensor>
{
public:
    enum class DataType : uint32_t
    {
        Undefined,
        Float16,
        Float32,
        Float64,
        Sint8,
        Sint16,
        Sint32,
        Sint64,
        Uint8,
        Uint16,
        Uint32,
        Uint64,
    };

    static bool IsFloatingPoint(DataType dataType);
    static bool IsSignedInteger(DataType dataType);
    static bool IsUnsignedInteger(DataType dataType);
    static bool IsInteger(DataType dataType);
    static uint64_t GetElementSizeInBytes(DataType dataType);

    Tensor(rad::Ref<Context> context);
    Tensor(rad::Ref<Context> context, DataType dataType,
        rad::Span<uint64_t> sizes, rad::Span<uint64_t> strides);
    ~Tensor();

    rad::Ref<Context> m_context;
    DataType m_dataType = DataType::Undefined;
    std::vector<uint64_t> m_sizes;
    std::vector<uint64_t> m_strides;

    // No gap between elements.
    bool m_isMemContiguous = false;
    // Elements are stored in memory sequentially without gaps,
    // and strides are in descending order (row major).
    bool m_isContiguous = false;

    // Indices of dimension in memory order.
    std::vector<uint64_t> m_memOrderIndices;

    uint64_t GetNumDimensions() const { return m_sizes.size(); }
    static uint64_t GetElementCount(rad::Span<uint64_t> sizes);
    uint64_t GetElementCount() const;
    // Contiguous, descending order (row major).
    static std::vector<uint64_t> GetDefaultStrides(rad::Span<uint64_t> sizes);
    static bool IsMemoryContiguous(rad::Span<uint64_t> sizes, rad::Span<uint64_t> strides);
    static bool IsContiguous(rad::Span<uint64_t> sizes, rad::Span<uint64_t> strides);
    // Return gap sizes of each dimension for non-contiguous tensor.
    std::vector<uint64_t> GetGapSizes() const;

    enum class MemoryLayout
    {
        Unknown,
        NCHW,
        NHWC,   // channel last format
        NCDHW,
        NDHWC,  // channel last format
    };
    static MemoryLayout GetMemoryLayout(rad::Span<uint64_t> sizes, rad::Span<uint64_t> strides);
    MemoryLayout m_memLayout = MemoryLayout::Unknown;

    // Different tensors may share the same storage, with different views.
    rad::Ref<Buffer> m_buffer;
    VkDeviceSize m_bufferOffset = 0;
    VkDeviceSize m_bufferSize = 0;

    static VkDeviceSize CalculateBufferSize(DataType dataType,
        rad::Span<uint64_t> sizes, rad::Span<uint64_t> strides);
    bool CreateBuffer(VkDeviceSize size);

    static rad::Ref<Tensor> CreateTensor(rad::Ref<Context> context,
        DataType dataType, rad::Span<uint64_t> sizes, rad::Span<uint64_t> strides = {});

    void FillFloat16(uint16_t value);
    void Fill(float value);
    void Fill(double value);
    void Fill(int64_t value);
    void Fill(uint64_t value);

    // Save tensor to file, in binary format:
    // uint32 dataType;
    // uint32 numDimension;
    // uint64 sizes[numDimension];
    // uint64 strides[numDimension];
    // uint64 dataSizeInBytes
    // byte data[]
    bool SaveToFile(std::string_view fileName);
    // Load buffer data from file.
    static rad::Ref<Tensor> CreateFromFile(rad::Ref<Context> context, std::string_view fileName);

    enum class DumpFormat
    {
        Readable,
        Hex,
    };
    std::string Dump(DumpFormat format, rad::Span<uint64_t> dumpOffsets, rad::Span<uint64_t> dumpSizes);
    std::string Dump(DumpFormat format);

}; // class Tensor

} // namespace vkpp
