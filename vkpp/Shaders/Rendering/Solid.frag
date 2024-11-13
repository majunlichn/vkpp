#version 450 core
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_fragment_shader_barycentric  : require

#include "Solid.glsl"

layout (set = 1, binding = 0) uniform sampler g_samplers[4];
layout (set = 1, binding = 1) uniform texture2D g_textures[];

layout(location = 0) in vec2 in_TexCoord;
layout (location = 0) out vec4 out_FragColor;

// https://wunkolo.github.io/post/2022/07/gl_ext_fragment_shader_barycentric-wireframe/
float GetWireFrameFactor(in float Thickness, in float Falloff)
{
    const vec3 BaryCoord = gl_BaryCoordEXT;

    const vec3 dBaryCoordX = dFdxFine(BaryCoord);
    const vec3 dBaryCoordY = dFdyFine(BaryCoord);
    const vec3 dBaryCoord  = sqrt(dBaryCoordX*dBaryCoordX + dBaryCoordY*dBaryCoordY);

    const vec3 dFalloff   = dBaryCoord * Falloff;
    const vec3 dThickness = dBaryCoord * Thickness;

    const vec3 Remap = smoothstep(dThickness, dThickness + dFalloff, BaryCoord);
    const float ClosestEdge = min(min(Remap.x, Remap.y), Remap.z);

    return 1.0 - ClosestEdge;
}

void main()
{
    vec4 color = texture(
        sampler2D(
            g_textures[g_meshInfo.baseColorTextureIndex],
            g_samplers[g_meshInfo.baseColorSamplerIndex]
        ),
        in_TexCoord,
        g_meshInfo.baseColorLodBias);

    if (g_meshInfo.shadeWireframe != 0)
    {
        float wireframeFactor = GetWireFrameFactor(1.0f, 1.5f);
        const vec4 wireframeColor = vec4(g_meshInfo.wireframeColor, 1.0f);
        out_FragColor = vec4(color) * (1.0f - wireframeFactor) + wireframeColor * wireframeFactor;
    }
    else
    {
        out_FragColor = vec4(color);
    }
}
