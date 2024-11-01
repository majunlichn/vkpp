#pragma once

#include <vkpp/Core/Common.h>
#include <spvgen.h>

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

class ShaderCompiler : public rad::RefCounted<ShaderCompiler>
{
public:
    ShaderCompiler();
    ~ShaderCompiler();

    std::vector<uint32_t> Assemble(
        VkShaderStageFlagBits stage, std::string_view fileName, std::string_view source);
    std::vector<uint32_t> Compile(
        VkShaderStageFlagBits stage, std::string_view fileName, std::string_view source,
        std::string_view entryPoint = "main", rad::Span<ShaderMacro> macros = {},
        uint32_t options = SpvGenOptionVulkanRules);
    std::vector<uint32_t> CompileFromFile(
        VkShaderStageFlagBits stage, std::string_view fileName,
        std::string_view entryPoint = "main", rad::Span<ShaderMacro> macros = {},
        uint32_t options = SpvGenOptionVulkanRules);

    const char* GetLog() const { return m_log.c_str(); }

private:
    std::string m_log;

}; // class ShaderCompiler

void SetShaderPath(std::string shaderPath);

} // namespace vkpp
