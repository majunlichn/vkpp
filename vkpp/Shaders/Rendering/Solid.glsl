

layout(set = 0, binding = 0) uniform FrameInfo
{
    mat4 viewProj;
} g_frameInfo;

layout(set = 0, binding = 1) uniform MeshInfo
{
    mat4 toWorld;
    uvec4 baseColorTextureIndex;
} g_meshInfo;
