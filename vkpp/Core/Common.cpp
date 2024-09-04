#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include <vkpp/Core/Common.h>

namespace vkpp
{

spdlog::logger* GetLogger()
{
    static std::shared_ptr<spdlog::logger> logger = rad::CreateLogger("vkpp");
    return logger.get();
}

void ReportError(VkResult result, const char* function, const char* file, uint32_t line)
{
    if (result < 0)
    {
        RAD_LOG(GetLogger(), err, "{} failed with VkResult={}({}, line {}).",
            function, string_VkResult(result), file, line);
        throw VulkanError(result);
    }
}

} // namespace vkpp
