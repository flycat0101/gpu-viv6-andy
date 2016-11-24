#version 450 core

layout (location = 0) in vec3 v_eye;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_texCoord;
layout (location = 3) in vec4 v_projCoord;

layout (location = 0) out float gl_FragDepth;

void main(void)
{
	gl_FragDepth = gl_FragCoord.z;
}
