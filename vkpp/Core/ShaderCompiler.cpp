#include <vkpp/Core/ShaderCompiler.h>
#include <vkpp/Core/Pipeline.h>
#include <rad/IO/File.h>
#include <rad/System/OS.h>

namespace vkpp
{

std::string g_shaderPath = rad::getenv("VKPP_SHADER_PATH");

ShaderCompiler::ShaderCompiler()
{
    if (!g_shaderPath.empty())
    {
        m_fileFinder.search_path().push_back(g_shaderPath);
    }
}

ShaderCompiler::~ShaderCompiler()
{
}

shaderc_shader_kind GetShaderKind(VkShaderStageFlagBits stage)
{
    switch (stage)
    {
    case VK_SHADER_STAGE_VERTEX_BIT: return shaderc_vertex_shader;
    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return shaderc_tess_control_shader;
    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return shaderc_tess_evaluation_shader;
    case VK_SHADER_STAGE_GEOMETRY_BIT: return shaderc_geometry_shader;
    case VK_SHADER_STAGE_FRAGMENT_BIT: return shaderc_fragment_shader;
    case VK_SHADER_STAGE_COMPUTE_BIT: return shaderc_compute_shader;
    case VK_SHADER_STAGE_RAYGEN_BIT_KHR: return shaderc_raygen_shader;
    case VK_SHADER_STAGE_ANY_HIT_BIT_KHR: return shaderc_anyhit_shader;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return shaderc_closesthit_shader;
    case VK_SHADER_STAGE_MISS_BIT_KHR: return shaderc_miss_shader;
    case VK_SHADER_STAGE_INTERSECTION_BIT_KHR: return shaderc_intersection_shader;
    case VK_SHADER_STAGE_CALLABLE_BIT_KHR: return shaderc_callable_shader;
    case VK_SHADER_STAGE_TASK_BIT_EXT: return shaderc_task_shader;
    case VK_SHADER_STAGE_MESH_BIT_EXT: return shaderc_mesh_shader;
    }
    return shaderc_glsl_infer_from_source;
}

std::string ShaderCompiler::PreprocessGLSL(
    VkShaderStageFlagBits stage, const std::string& fileName, const std::string& source,
    const std::string& entryPoint, rad::Span<ShaderMacro> macros)
{
    shaderc::CompileOptions options;

    for (const auto& macro : macros)
    {
        options.AddMacroDefinition(macro.m_name, macro.m_definition);
    }
    if (!rad::StrEqual(entryPoint, "main"))
    {
        options.AddMacroDefinition(entryPoint, "main");
    }

    shaderc::PreprocessedSourceCompilationResult result =
        m_compiler.PreprocessGlsl(source, GetShaderKind(stage), fileName.c_str(), options);
    if (result.GetCompilationStatus() == shaderc_compilation_status_success)
    {
        return { result.cbegin(), result.cend() };
    }
    else
    {
        m_log = result.GetErrorMessage();
        return {};
    }
}

std::vector<uint32_t> ShaderCompiler::CompileGLSL(
    VkShaderStageFlagBits stage, const std::string& fileName, const std::string& source,
    const std::string& entryPoint, rad::Span<ShaderMacro> macros)
{
    shaderc::CompileOptions options;
    for (const ShaderMacro& macro : macros)
    {
        options.AddMacroDefinition(macro.m_name, macro.m_definition);
    }
    if (!rad::StrEqual(entryPoint, "main"))
    {
        options.AddMacroDefinition(entryPoint, "main");
    }

    std::unique_ptr<FileIncluder> includer(
        RAD_NEW FileIncluder(&m_fileFinder));
    options.SetIncluder(std::move(includer));

    shaderc::SpvCompilationResult result =
        m_compiler.CompileGlslToSpv(source, GetShaderKind(stage), fileName.c_str(), options);

    if (result.GetCompilationStatus() == shaderc_compilation_status_success)
    {
        return { result.cbegin(), result.cend() };
    }
    else
    {
        m_log = result.GetErrorMessage();
        VKPP_LOG(err, "CompileGLSL \"{}\":\n {}", fileName, m_log);
        return {};
    }
}

std::vector<uint32_t> ShaderCompiler::CompileGLSLFromFile(
    VkShaderStageFlagBits stage, const std::string& fileName,
    const std::string& entryPoint, rad::Span<ShaderMacro> macros)
{
    std::string path(fileName);
    if (!fileName.empty())
    {
        if (!pystring::os::path::isabs(path))
        {
            path = g_shaderPath + "/" + path;
        }
    }
    return CompileGLSL(stage, path, rad::File::ReadAll(path), entryPoint, macros);
}

} // namespace vkpp
