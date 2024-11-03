#pragma once

#include <vkpp/Core/Common.h>
#include <shaderc/shaderc.hpp>
#include "ShaderIncluder.h"

namespace vkpp
{

struct ShaderMacro
{
    ShaderMacro()
    {
    }

    ShaderMacro(std::string_view name)
    {
        this->m_name = name;
    }

    ShaderMacro(std::string_view name, std::string_view definition)
    {
        this->m_name = name;
        this->m_definition = definition;
    }

    template<typename T>
    ShaderMacro(std::string_view name, T definition)
    {
        this->m_name = name;
        this->m_definition = std::to_string(definition);
    }

    std::string m_name;
    std::string m_definition;

}; // class ShaderMacro

extern std::string g_shaderPath;

class ShaderCompiler : public rad::RefCounted<ShaderCompiler>
{
public:
    ShaderCompiler();
    ~ShaderCompiler();

    void AddIncludeDir(std::string includeDir)
    {
        m_fileFinder.search_path().push_back(std::move(includeDir));
    }

    std::string PreprocessGLSL(
        VkShaderStageFlagBits stage, const std::string& fileName, const std::string& source,
        const std::string& entryPoint = "main", rad::Span<ShaderMacro> macros = {});
    std::vector<uint32_t> CompileGLSL(
        VkShaderStageFlagBits stage, const std::string& fileName, const std::string& source,
        const std::string& entryPoint = "main", rad::Span<ShaderMacro> macros = {});
    std::vector<uint32_t> CompileGLSLFromFile(
        VkShaderStageFlagBits stage, const std::string& fileName,
        const std::string& entryPoint, rad::Span<ShaderMacro> macros);

private:
    shaderc::Compiler m_compiler;
    std::string m_log;
    FileFinder m_fileFinder;

}; // class ShaderCompiler

} // namespace vkpp
