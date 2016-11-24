#version 450 core

layout (binding = 0) uniform sampler2D u_texture; 

layout (location = 0) in vec2 v_texCoord;
layout (location = 1) in vec4 v_color;

layout (location = 0) out vec4 ob_fragColor;

void main(void)
{
	ob_fragColor = vec4(1.0, 1.0, 1.0, texture(u_texture, v_texCoord).r) * v_color;
}
