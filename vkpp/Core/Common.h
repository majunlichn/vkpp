#pragma once

#include <rad/Core/Integer.h>
#include <rad/Core/Memory.h>
#include <rad/Core/RefCounted.h>
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

class VulkanError : public std::exception
{
public:
    VulkanError(VkResult result) : m_result(result) {}
    const char* what() const noexcept override { return string_VkResult(m_result); }
private:
    VkResult m_result;
}; // class VulkanError

spdlog::logger* GetLogger();

// Check Vulkan return code and throw VulkanError if result < 0.
void ReportError(VkResult result, const char* function, const char* file, uint32_t line);
#define VK_CHECK(func) \
    do { const VkResult result = func; ReportError(result, #func, __FILE__, __LINE__); } while(0)

} // namespace vkpp
