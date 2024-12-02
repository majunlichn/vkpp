#pragma once

#include <vkpp/Core/Context.h>

namespace vkpp
{

class ElementWiseUnary
{
public:
    rad::Ref<DescriptorSetLayout> m_descSetLayout;
    rad::Ref<PipelineLayout> m_pipelineLayout;
    rad::Ref<Pipeline> m_pipeline;

    rad::Ref<DescriptorPool> m_descPool;
    rad::Ref<DescriptorSet> m_descSet;

}; // class ElementWiseUnary

} // namespace vkpp
