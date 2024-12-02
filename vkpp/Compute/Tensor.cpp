#include <vkpp/Compute/Tensor.h>
#include <rad/Core/Float16.h>
#include <rad/Core/Sort.h>
#include <rad/IO/File.h>

#include <algorithm>
#include <numeric>

namespace vkpp
{

bool Tensor::IsFloatingPoint(DataType dataType)
{
    switch (dataType)
    {
    case DataType::Float16:
    case DataType::Float32:
    case DataType::Float64:
        return true;
    }
    return false;
}

bool Tensor::IsSignedInteger(DataType dataType)
{
    switch (dataType)
    {
    case DataType::Sint8:
    case DataType::Sint16:
    case DataType::Sint32:
    case DataType::Sint64:
        return true;
    }
    return false;
}

bool Tensor::IsUnsignedInteger(DataType dataType)
{
    switch (dataType)
    {
    case DataType::Uint8:
    case DataType::Uint16:
    case DataType::Uint32:
    case DataType::Uint64:
        return true;
    }
    return false;
}

bool Tensor::IsInteger(DataType dataType)
{
    return IsSignedInteger(dataType) || IsUnsignedInteger(dataType);
}

uint64_t Tensor::GetElementSizeInBytes(DataType dataType)
{
    switch (dataType)
    {
    case DataType::Float16: return 2;
    case DataType::Float32: return 4;
    case DataType::Float64: return 8;
    case DataType::Sint8:   return 1;
    case DataType::Sint16:  return 2;
    case DataType::Sint32:  return 4;
    case DataType::Sint64:  return 8;
    case DataType::Uint8:   return 1;
    case DataType::Uint16:  return 2;
    case DataType::Uint32:  return 4;
    case DataType::Uint64:  return 8;
    }
    return uint64_t(0);
}

Tensor::Tensor(rad::Ref<Context> context) :
    m_context(std::move(context))
{
}

Tensor::Tensor(rad::Ref<Context> context, DataType dataType,
    rad::Span<uint64_t> sizes, rad::Span<uint64_t> strides) :
    m_context(std::move(context)),
    m_dataType(dataType),
    m_sizes(sizes),
    m_strides(strides)
{
    if (m_strides.empty())
    {
        m_strides = GetDefaultStrides(sizes);
        m_isMemContiguous = true;
        m_isContiguous = true;
    }
    else
    {
        assert(sizes.size() == strides.size());
        m_isMemContiguous = IsMemoryContiguous(sizes, strides);
        m_isContiguous = IsContiguous(sizes, strides);
        m_memOrderIndices = rad::SortIndices(strides, std::greater<uint64_t>());
    }

    m_memLayout = GetMemoryLayout(m_sizes, m_strides);
}

Tensor::~Tensor()
{
}

uint64_t Tensor::GetElementCount(rad::Span<uint64_t> sizes)
{
    return std::accumulate(sizes.begin(), sizes.end(), uint64_t(1),
        std::multiplies<uint64_t>());
}

uint64_t Tensor::GetElementCount() const
{
    assert(m_sizes.size() == m_strides.size());
    return GetElementCount(m_sizes);
}

std::vector<uint64_t> Tensor::GetDefaultStrides(rad::Span<uint64_t> sizes)
{
    std::vector<uint64_t> strides;
    assert(sizes.size() > 0);
    if (sizes.size() > 0)
    {
        strides.resize(sizes.size(), 0);
        strides.back() = 1;
        std::partial_sum(sizes.rbegin(), sizes.rend() - 1, strides.rbegin() + 1,
            std::multiplies<uint64_t>());
    }
    return strides;
}

bool Tensor::IsMemoryContiguous(rad::Span<uint64_t> sizes, rad::Span<uint64_t> strides)
{
    assert(sizes.size() > 0);
    assert(sizes.size() == strides.size());
    if (sizes.empty() || (sizes.size() != strides.size()))
    {
        return false;
    }

    uint64_t indexOfLastElement = 0;
    for (size_t i = 0; i < sizes.size(); ++i)
    {
        if (sizes[i] == 0)
        {
            continue;
        }
        indexOfLastElement += (sizes[i] - 1) * strides[i];
    }
    return ((indexOfLastElement + 1) == GetElementCount(sizes));
}

bool Tensor::IsContiguous(rad::Span<uint64_t> sizes, rad::Span<uint64_t> strides)
{
    assert(sizes.size() > 0);
    assert(sizes.size() == strides.size());
    if (sizes.empty() || (sizes.size() != strides.size()))
    {
        return false;
    }

    if (strides.back() != 1)
    {
        return false;
    }
    for (size_t i = 0; i < strides.size() - 1; ++i)
    {
        if (strides[i] != strides[i + 1] * sizes[i + 1])
        {
            return false;
        }
    }
    return true;
}

std::vector<uint64_t> Tensor::GetGapSizes() const
{
    std::vector<uint64_t> gapSizes(m_sizes.size(), 0);
    auto defaultStrides = GetDefaultStrides(m_sizes);
    for (size_t i = 0; i < gapSizes.size(); ++i)
    {
        uint64_t dimIndex = m_memOrderIndices[i];
        gapSizes[dimIndex] = m_strides[dimIndex] - defaultStrides[i];
    }
    return gapSizes;
}

Tensor::MemoryLayout Tensor::GetMemoryLayout(rad::Span<uint64_t> sizes, rad::Span<uint64_t> strides)
{
    if (IsMemoryContiguous(sizes, strides))
    {
        if ((sizes.size() == 4) && (strides.size() == 4))
        {
            if ((strides[0] > strides[1]) && (strides[1] > strides[2]) &&
                (strides[2] > strides[3]))
            {
                return MemoryLayout::NCHW;
            }
            else if ((strides[0] > strides[2]) && (strides[2] > strides[3]) &&
                (strides[3] > strides[1]))
            {
                return MemoryLayout::NHWC;
            }
        }
        else if ((sizes.size() == 5) && (strides.size() == 5))
        {
            if ((strides[0] > strides[1]) && (strides[1] > strides[2]) &&
                (strides[2] > strides[3]) && (strides[3] > strides[4]))
            {
                return MemoryLayout::NCDHW;
            }
            else if ((strides[0] > strides[2]) && (strides[2] > strides[3]) &&
                (strides[3] > strides[4]) && (strides[4] > strides[1]))
            {
                return MemoryLayout::NDHWC;
            }
        }
    }
    return MemoryLayout::Unknown;
}

VkDeviceSize Tensor::CalculateBufferSize(DataType dataType,
    rad::Span<uint64_t> sizes, rad::Span<uint64_t> strides)
{
    VkDeviceSize sizeInBytes = 0;
    if (strides.empty())
    {
        // Default strides, elements are contiguous.
        sizeInBytes = GetElementCount(sizes) * GetElementSizeInBytes(dataType);
    }
    else
    {
        uint64_t indexOfLastElement = 0;
        for (size_t i = 0; i < sizes.size(); ++i)
        {
            if (sizes[i] == 0)
            {
                continue;
            }
            indexOfLastElement += (sizes[i] - 1) * strides[i];
        }
        sizeInBytes = (indexOfLastElement + 1) * GetElementSizeInBytes(dataType);
    }
    sizeInBytes = rad::RoundUpToMultiple<VkDeviceSize>(sizeInBytes, 4);
    return sizeInBytes;
}

bool Tensor::CreateBuffer(VkDeviceSize size)
{
    m_buffer = m_context->GetDevice()->CreateStorageBuffer(size);
    m_bufferOffset = 0;
    m_bufferSize = m_buffer->GetSize();
    return true;
}

rad::Ref<Tensor> Tensor::CreateTensor(rad::Ref<Context> context, DataType dataType,
    rad::Span<uint64_t> sizes, rad::Span<uint64_t> strides)
{
    rad::Ref<Tensor> tensor = RAD_NEW Tensor(context, dataType, sizes, strides);
    tensor->CreateBuffer(CalculateBufferSize(dataType, sizes, strides));
    return tensor;
}

// TODO: Fill methods should be implemented with compute pipeline (ElementWiseUnary).

void Tensor::FillFloat16(uint16_t value)
{
    if (m_dataType == DataType::Float16)
    {
        std::vector<uint16_t> hostBuffer(m_bufferSize / sizeof(uint16_t), value);
        m_context->WriteBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    }
}

void Tensor::Fill(float value)
{
    if (m_dataType == DataType::Float16)
    {
        std::vector<uint16_t> hostBuffer(m_bufferSize / sizeof(uint16_t),
            rad::fp16_ieee_from_fp32_value(value));
        m_context->WriteBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    }
    else if (m_dataType == DataType::Float32)
    {
        std::vector<float> hostBuffer(m_bufferSize / sizeof(float), value);
        m_context->WriteBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    }
    else if (m_dataType == DataType::Float64)
    {
        static_assert(sizeof(double) == 8);
        std::vector<double> hostBuffer(m_bufferSize / sizeof(double), value);
        m_context->WriteBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    }
}

void Tensor::Fill(double value)
{
    if (m_dataType == DataType::Float64)
    {
        static_assert(sizeof(double) == 8);
        std::vector<double> hostBuffer(m_bufferSize / sizeof(double), value);
        m_context->WriteBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    }
    else
    {
        Fill(static_cast<float>(value));
    }
}

void Tensor::Fill(int64_t value)
{
    if (m_dataType == DataType::Sint8)
    {
        std::vector<int8_t> hostBuffer(m_bufferSize / sizeof(int8_t), int8_t(value));
        m_context->WriteBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    }
    else if (m_dataType == DataType::Sint16)
    {
        std::vector<int16_t> hostBuffer(m_bufferSize / sizeof(int16_t), int16_t(value));
        m_context->WriteBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    }
    else if (m_dataType == DataType::Sint32)
    {
        std::vector<int32_t> hostBuffer(m_bufferSize / sizeof(int32_t), int32_t(value));
        m_context->WriteBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    }
    else if (m_dataType == DataType::Sint64)
    {
        std::vector<int64_t> hostBuffer(m_bufferSize / sizeof(int64_t), int64_t(value));
        m_context->WriteBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    }
    else
    {
        Fill(static_cast<uint64_t>(value));
    }
}

void Tensor::Fill(uint64_t value)
{
    if (m_dataType == DataType::Uint8)
    {
        std::vector<uint8_t> hostBuffer(m_bufferSize / sizeof(uint8_t), uint8_t(value));
        m_context->WriteBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    }
    else if (m_dataType == DataType::Uint16)
    {
        std::vector<uint16_t> hostBuffer(m_bufferSize / sizeof(uint16_t), uint16_t(value));
        m_context->WriteBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    }
    else if (m_dataType == DataType::Uint32)
    {
        std::vector<uint32_t> hostBuffer(m_bufferSize / sizeof(uint32_t), uint32_t(value));
        m_context->WriteBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    }
    else if (m_dataType == DataType::Uint64)
    {
        std::vector<uint64_t> hostBuffer(m_bufferSize / sizeof(uint64_t), uint64_t(value));
        m_context->WriteBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    }
    else
    {
        Fill(static_cast<int64_t>(value));
    }
}

bool Tensor::SaveToFile(std::string_view fileName)
{
    rad::File file;
    if (file.Open(fileName, "wb"))
    {
        file.Write(&m_dataType, sizeof(m_dataType));
        uint32_t numDimensions = static_cast<uint32_t>(m_sizes.size());
        file.Write(&numDimensions, sizeof(numDimensions));
        file.Write(m_sizes.data(), sizeof(uint64_t), m_sizes.size());
        file.Write(m_strides.data(), sizeof(uint64_t), m_strides.size());
        static_assert(sizeof(VkDeviceSize) == sizeof(uint64_t));
        assert(m_bufferSize == CalculateBufferSize(m_dataType, m_sizes, m_strides));
        file.Write(&m_bufferSize, sizeof(m_bufferSize));
        std::vector<uint8_t> hostBuffer(m_bufferSize);
        m_context->ReadBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
        file.Write(hostBuffer.data(), hostBuffer.size());
        file.Close();
        return true;
    }
    return false;
}

rad::Ref<Tensor> Tensor::CreateFromFile(rad::Ref<Context> context, std::string_view fileName)
{
    rad::Ref<Tensor> tensor;
    rad::File file;
    if (file.Open(fileName, "rb"))
    {
        DataType dataType = DataType::Undefined;
        file.Read(&dataType, sizeof(dataType));
        uint32_t numDimensions = 0;
        file.Read(&numDimensions, sizeof(numDimensions));
        std::vector<uint64_t> sizes(numDimensions);
        std::vector<uint64_t> strides(numDimensions);
        file.Read(sizes.data(), sizeof(uint64_t), sizes.size());
        file.Read(strides.data(), sizeof(uint64_t), strides.size());
        uint64_t elementCount = GetElementCount(sizes);
        uint64_t bufferSize = 0;
        file.Read(&bufferSize, sizeof(bufferSize));
        if ((dataType != DataType::Undefined) && (numDimensions > 0) &&
            (elementCount > 0) &&
            (bufferSize == CalculateBufferSize(dataType, sizes, strides)))
        {
            tensor = CreateTensor(context, dataType, sizes, strides);
            std::vector<uint8_t> hostBuffer(bufferSize);
            file.Read(hostBuffer.data(), static_cast<uint64_t>(bufferSize));
            context->WriteBuffer(tensor->m_buffer.get(), hostBuffer.data());
            file.Close();
        }
    }
    return tensor;
}

std::string DumpElementReadable(Tensor::DataType dataType, const void* ptr)
{
    if (dataType == Tensor::DataType::Float16)
    {
        float val = rad::fp16_ieee_to_fp32_value(*static_cast<const uint16_t*>(ptr));
        return std::format("{:11.4f}", val);
    }
    else if (dataType == Tensor::DataType::Float32)
    {
        float val = *static_cast<const float*>(ptr);
        if (val < float(INT32_MAX))
        {
            return std::format("{:18.6f}", val);
        }
        else
        {
            return std::format("{:18.6e}", val);
        }
    }
    else if (dataType == Tensor::DataType::Float64)
    {
        double val = *static_cast<const double*>(ptr);
        if (val < double(INT32_MAX))
        {
            return std::format("{:18.6f}", val);
        }
        else
        {
            return std::format("{:18.6e}", val);
        }
    }
    else if (dataType == Tensor::DataType::Sint8)
    {
        int8_t val = *static_cast<const int8_t*>(ptr);
        return std::format("{:4}", val);
    }
    else if (dataType == Tensor::DataType::Sint16)
    {
        int16_t val = *static_cast<const int16_t*>(ptr);
        return std::format("{:6}", val);
    }
    else if (dataType == Tensor::DataType::Sint32)
    {
        int32_t val = *static_cast<const int32_t*>(ptr);
        return std::format("{:11}", val);
    }
    else if (dataType == Tensor::DataType::Sint64)
    {
        int64_t val = *static_cast<const int64_t*>(ptr);
        return std::format("{:20}", val);
    }
    else if (dataType == Tensor::DataType::Uint8)
    {
        uint8_t val = *static_cast<const uint8_t*>(ptr);
        return std::format("{:3}", val);
    }
    else if (dataType == Tensor::DataType::Uint16)
    {
        uint16_t val = *static_cast<const uint16_t*>(ptr);
        return std::format("{:5}", val);
    }
    else if (dataType == Tensor::DataType::Uint32)
    {
        uint32_t val = *static_cast<const uint32_t*>(ptr);
        return std::format("{:10}", val);
    }
    else if (dataType == Tensor::DataType::Uint64)
    {
        uint64_t val = *static_cast<const uint64_t*>(ptr);
        return std::format("{:19}", val);
    }
    else
    {
        return std::string();
    }
}

std::string DumpElementHex(Tensor::DataType dataType, const void* ptr)
{
    uint64_t elemSize = Tensor::GetElementSizeInBytes(dataType);
    if (elemSize == 1)
    {
        uint8_t val = *static_cast<const uint8_t*>(ptr);
        return std::format("0x{:02X}", val);
    }
    else if (elemSize == 2)
    {
        uint16_t val = *static_cast<const uint16_t*>(ptr);
        return std::format("0x{:04X}", val);
    }
    else if (elemSize == 4)
    {
        uint32_t val = *static_cast<const uint32_t*>(ptr);
        return std::format("0x{:08X}", val);
    }
    else if (elemSize == 8)
    {
        uint64_t val = *static_cast<const uint64_t*>(ptr);
        return std::format("0x{:016X}", val);
    }
    else
    {
        return std::string();
    }
}

std::string Tensor::Dump(DumpFormat format, rad::Span<uint64_t> dumpOffsets, rad::Span<uint64_t> dumpSizes)
{
    assert(m_sizes.size() == dumpOffsets.size());
    assert(m_sizes.size() == dumpSizes.size());
    std::vector<uint8_t*> hostBuffer(m_bufferSize);
    m_context->ReadBuffer(m_buffer.get(), hostBuffer.data(), m_bufferOffset, m_bufferSize);
    std::string str;
    uint64_t numDimensions = m_sizes.size();
    uint64_t elemSize = GetElementSizeInBytes(m_dataType);
    if (numDimensions == 4)
    {
        for (uint64_t n = dumpOffsets[0]; n < dumpOffsets[0] + dumpSizes[0]; ++n)
        {
            for (uint64_t c = dumpOffsets[1]; c < dumpOffsets[1] + dumpSizes[1]; ++c)
            {
                str += std::format("# N={}; C={}; H={}:{}; W={}:{}\n", n, c,
                    dumpOffsets[2], dumpOffsets[2] + dumpSizes[2],
                    dumpOffsets[3], dumpOffsets[3] + dumpSizes[3]);
                for (uint64_t h = dumpOffsets[2]; h < dumpOffsets[2] + dumpSizes[2]; ++h)
                {
                    for (uint64_t w = dumpOffsets[3]; w < dumpOffsets[3] + dumpSizes[3]; ++w)
                    {
                        uint64_t index = n * m_strides[0] + c * m_strides[1] +
                            h * m_strides[2] + w * m_strides[3];
                        uint64_t offset = index * elemSize;
                        if (format == DumpFormat::Readable)
                        {
                            str += DumpElementReadable(m_dataType, hostBuffer.data() + offset) + ", ";
                        }
                        else
                        {
                            str += DumpElementHex(m_dataType, hostBuffer.data() + offset) + ", ";
                        }
                    }
                    str.pop_back(); // pop the last whitespace
                    str += "\n";    // next row
                }
            }
        }
    }
    else if (numDimensions == 5)
    {
        for (uint64_t n = dumpOffsets[0]; n < dumpOffsets[0] + dumpSizes[0]; ++n)
        {
            for (uint64_t c = dumpOffsets[1]; c < dumpOffsets[1] + dumpSizes[1]; ++c)
            {
                for (uint64_t d = dumpOffsets[2]; d < dumpOffsets[2] + dumpSizes[2]; ++d)
                {
                    str += std::format("# N={}; C={}; D={}; H={}:{}; W={}:{}\n", n, c, d,
                        dumpOffsets[3], dumpOffsets[3] + dumpSizes[3],
                        dumpOffsets[4], dumpOffsets[4] + dumpSizes[4]);
                    for (uint64_t h = dumpOffsets[3]; h < dumpOffsets[3] + dumpSizes[3]; ++h)
                    {
                        for (uint64_t w = dumpOffsets[4]; w < dumpOffsets[4] + dumpSizes[4]; ++w)
                        {
                            uint64_t index = n * m_strides[0] + c * m_strides[1] + d * m_strides[2] +
                                h * m_strides[3] + w * m_strides[4];
                            uint64_t offset = index * elemSize;
                            if (format == DumpFormat::Readable)
                            {
                                str += DumpElementReadable(m_dataType, hostBuffer.data() + offset) + ", ";
                            }
                            else
                            {
                                str += DumpElementHex(m_dataType, hostBuffer.data() + offset) + ", ";
                            }
                        }
                        str.pop_back(); // pop the last whitespace
                        str += "\n";    // next row
                    }
                }
            }
        }
    }
    return str;
}

std::string Tensor::Dump(DumpFormat format)
{
    std::vector<uint64_t> dumpOffsets(m_sizes.size(), 0);
    std::vector<uint64_t> dumpSizes(m_sizes);
    return Dump(format, dumpOffsets, dumpSizes);
}

} // namespace vkpp
