#version 450 core

layout (location = 0) out vec2 out_TexCoord;

void main()
{
    vec4 vertices[3] = vec4[3](
        vec4(-1.0f, 1.0f, 0.0f, 1.0f),
        vec4(-1.0f,-3.0f, 0.0f,-1.0f),
        vec4( 3.0f, 1.0f, 2.0f, 1.0f)
    );
    vec4 vertex = vertices[gl_VertexIndex & 3];
    gl_Position = vec4(vertex.xy, 0.0f, 1.0f);
    out_TexCoord = vertex.zw;
}
