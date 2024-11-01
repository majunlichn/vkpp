#define SPVGEN_STATIC_LIB
#include <vkpp/Core/ShaderCompiler.h>
#include <vkpp/Core/Pipeline.h>
#include <rad/IO/File.h>
#include <rad/System/OS.h>

namespace vkpp
{

ShaderCompiler::ShaderCompiler()
{
    if (!InitSpvGen())
    {
        VKPP_LOG(err, "InitSpvGen failed!");
    }
}

ShaderCompiler::~ShaderCompiler()
{
    FinalizeSpvgen();
}

std::vector<uint32_t> ShaderCompiler::Assemble(
    VkShaderStageFlagBits stage, std::string_view fileName, std::string_view source)
{
    std::vector<uint32_t> binary;
    unsigned int* buffer = nullptr;
    m_log.clear();
    const char* log = nullptr;
    size_t bufferSize = spvAssembleSpirv(
        source.data(), static_cast<uint32_t>(source.size()), buffer, &log);
    if (buffer && (bufferSize > 0) && ((bufferSize % 4) == 0))
    {
        binary.resize(bufferSize / 4);
        memcpy(binary.data(), buffer, bufferSize);
        m_log.clear();
    }
    else
    {
        m_log = log;
    }
    spvFreeBuffer(buffer);
    return binary;
}

std::vector<uint32_t> ShaderCompiler::Compile(
    VkShaderStageFlagBits stage, std::string_view fileName, std::string_view source,
    std::string_view entryPoint, rad::Span<ShaderMacro> macros, uint32_t options)
{
    std::vector<uint32_t> binary;
    SpvGenStage spvStage = SpvGenStageInvalid;
    int sourceStringCount = 1;
    const char* stageSources[] = { source.data() };
    const char* stageFiles[] = { fileName.data() };
    const char* const* sourceList[] = { stageSources };
    const char* const* fileList[] = { stageFiles };
    const char* entryPoints[] = { entryPoint.data() };
    void* program = nullptr;
    m_log.clear();
    const char* log = nullptr;
    switch (stage)
    {
    case VK_SHADER_STAGE_VERTEX_BIT:                    spvStage = SpvGenStageVertex;               break;
    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:      spvStage = SpvGenStageTessControl;          break;
    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:   spvStage = SpvGenStageTessEvaluation;       break;
    case VK_SHADER_STAGE_GEOMETRY_BIT:                  spvStage = SpvGenStageGeometry;             break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:                  spvStage = SpvGenStageFragment;             break;
    case VK_SHADER_STAGE_COMPUTE_BIT:                   spvStage = SpvGenStageCompute;              break;
    case VK_SHADER_STAGE_RAYGEN_BIT_KHR:                spvStage = SpvGenStageRayTracingRayGen;     break;
    case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:               spvStage = SpvGenStageRayTracingAnyHit;     break;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:           spvStage = SpvGenStageRayTracingClosestHit; break;
    case VK_SHADER_STAGE_MISS_BIT_KHR:                  spvStage = SpvGenStageRayTracingMiss;       break;
    case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:          spvStage = SpvGenStageRayTracingIntersect;  break;
    case VK_SHADER_STAGE_CALLABLE_BIT_KHR:              spvStage = SpvGenStageRayTracingCallable;   break;
    case VK_SHADER_STAGE_TASK_BIT_NV:                   spvStage = SpvGenStageTask;                 break;
    case VK_SHADER_STAGE_MESH_BIT_NV:                   spvStage = SpvGenStageMesh;                 break;
    }

    options |= SpvGenOptionVulkanRules;

    bool result = spvCompileAndLinkProgramEx(
        1, &spvStage,
        &sourceStringCount, sourceList, fileList, entryPoints,
        &program, &log,
        options);

    if (result)
    {
        const uint32_t* buffer = nullptr;
        int binarySize = spvGetSpirvBinaryFromProgram(program, 0, &buffer);
        assert((binarySize > 0) && ((binarySize % 4) == 0));
        binary.resize(binarySize / sizeof(uint32_t));
        memcpy(binary.data(), buffer, binarySize);
        m_log.clear();
    }
    else
    {
        m_log = log;
    }

    spvFreeBuffer(program);
    return binary;
}

static std::string g_shaderPath = rad::getenv("VKPP_SHADER_PATH");

void SetShaderPath(std::string shaderPath)
{
    g_shaderPath = std::move(shaderPath);
}

std::vector<uint32_t> ShaderCompiler::CompileFromFile(
    VkShaderStageFlagBits stage, std::string_view fileName,
    std::string_view entryPoint, rad::Span<ShaderMacro> macros, uint32_t options)
{
    std::string path(fileName);
    if (!fileName.empty())
    {
        if (!pystring::os::path::isabs(path))
        {
            path = g_shaderPath + "/" + path;
        }
    }
    return Compile(stage, path, rad::File::ReadAll(path), entryPoint, macros);
}

} // namespace vkpp
