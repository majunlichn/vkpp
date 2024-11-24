#version 450 core

layout (set = 0, binding = 0) uniform sampler2D g_colorBuffer;
layout (set = 0, binding = 1) uniform sampler2D g_overlay;

layout (location = 0) in vec2 in_TexCoord;
layout (location = 0) out vec4 out_FragColor;

void main()
{
    vec4 color = textureLod(g_colorBuffer, in_TexCoord, 0);
    vec4 overlay = textureLod(g_overlay, in_TexCoord, 0);
    out_FragColor.rgb = color.rgb * (1.0f - overlay.a) + overlay.rgb;
    out_FragColor.a = 1.0f;
}
