#version 450 core

layout (binding = 6) uniform samplerCube u_cubemap;

layout (location = 0) in vec3 v_ray;

layout (location = 0) out vec4 ob_fragColor;

void main(void)
{
	ob_fragColor = texture(u_cubemap, normalize(v_ray));
}
