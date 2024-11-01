#version 450 core
#extension GL_GOOGLE_include_directive : enable

#include "Solid.glsl"

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec4 in_Tangent;
layout(location = 3) in vec2 in_TexCoord;

layout(location = 0) out vec2 out_TexCoord;

void main()
{
    vec4 worldPosition = g_meshInfo.toWorld * vec4(in_Position, 1.0f);
    gl_Position = g_frameInfo.viewProj * worldPosition;
    out_TexCoord = in_TexCoord;
}
