#version 450 core

layout(push_constant, std140) uniform _u_font {
        mat4 vertexTransform;
        mat3 texCoordTransform;
        vec4 color;
} u_font;

layout (location = 0) in vec4 a_vertex;

layout (location = 0) out vec2 v_texCoord;
layout (location = 1) out flat vec4 v_color;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() 
{
    v_texCoord = (u_font.texCoordTransform * vec3(a_vertex.xy, 1.0)).xy;

    v_color = u_font.color;

	gl_Position = u_font.vertexTransform * a_vertex;
}
