#pragma once

#include <rad/Core/Integer.h>
#include <rad/Core/Memory.h>
#include <rad/Core/RefCounted.h>
#include <rad/Core/String.h>
#include <rad/Core/TypeTraits.h>
#include <rad/Container/Span.h>
#include <rad/IO/Logging.h>
#include <exception>

#define VK_NO_PROTOTYPES
#define VK_ENABLE_BETA_EXTENSIONS
#include <volk.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/utility/vk_format_utils.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>

namespace vkpp
{

enum QueueFamily : uint32_t
{
    QueueFamilyUniversal,     // Universal Engine supports all features (Graphics or Compute).
    QueueFamilyCompute,       // Async Compute Engine (ACE)
    QueueFamilyTransfer,      // DMA
    QueueFamilyVideoDecode,
    QueueFamilyVideoEncode,
    QueueFamilyOpticalFlow,
    QueueFamilyCount,
};

class Error : public std::exception
{
public:
    Error(VkResult result) : m_result(result) {}
    const char* what() const noexcept override { return string_VkResult(m_result); }
private:
    VkResult m_result;
}; // class Error

spdlog::logger* GetLogger();
#define VKPP_LOG(Level, ...) RAD_LOG(vkpp::GetLogger(), Level, __VA_ARGS__)

// Check Vulkan return code and throw Error if result < 0.
void ReportError(VkResult result, const char* function, const char* file, uint32_t line);
#define VK_CHECK(func) \
    do { const VkResult result = func; ReportError(result, #func, __FILE__, __LINE__); } while(0)

#define VK_STRUCTURE_CHAIN_BEGIN(Head) auto Head##ChainNext = &Head.pNext;
#define VK_STRUCTURE_CHAIN_ADD(Head, Next) do { *Head##ChainNext = &Next; Head##ChainNext = &Next.pNext; } while(0)
#define VK_STRUCTURE_CHAIN_END(Head) do { *Head##ChainNext = nullptr; } while(0)

bool IsVersionMatchOrGreater(uint32_t version, uint32_t major, uint32_t minor, uint32_t patch);
std::vector<VkLayerProperties> EnumerateInstanceLayers();
std::vector<VkExtensionProperties> EnumerateInstanceExtensions(const char* layerName = nullptr);
bool HasLayer(rad::Span<VkLayerProperties> extensions, std::string_view name);
bool HasExtension(rad::Span<VkExtensionProperties> extensions, std::string_view name);
uint32_t GetMaxMipLevel(uint32_t width, uint32_t height);
uint32_t GetMaxMipLevel(uint32_t width, uint32_t height, uint32_t depth);
VkImageAspectFlags GetImageAspectFromFormat(VkFormat format);

class Instance;
class PhysicalDevice;
class Device;
class Queue;
class CommandPool;
class CommandBuffer;
class Fence;
class Semaphore;
class Event;
class RenderPass;
class Framebuffer;
class ShaderModule;
class PipelineLayout;
class Pipeline;
class GraphicsPipeline;
class ComputePipeline;
class Buffer;
class BufferView;
class Image;
class ImageView;
class Sampler;
class DescriptorPool;
class DescriptorSetLayout;
class DescriptorSet;
class Surface;
class Swapchain;

} // namespace vkpp

#define VKPP_DISABLE_COPY_AND_MOVE(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete; \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete;
