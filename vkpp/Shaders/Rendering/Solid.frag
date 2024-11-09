#version 450 core
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "Solid.glsl"

layout (set = 1, binding = 0) uniform sampler g_samplers[4];
layout (set = 1, binding = 1) uniform texture2D g_textures[];

layout(location = 0) in vec2 in_TexCoord;
layout (location = 0) out vec4 out_FragColor;

void main()
{
    vec4 color = texture(
        sampler2D(
            g_textures[g_meshInfo.baseColorTextureIndex],
            g_samplers[g_meshInfo.baseColorSamplerIndex]
        ),
        in_TexCoord,
        g_meshInfo.baseColorLodBias);
    out_FragColor = vec4(color);
}
