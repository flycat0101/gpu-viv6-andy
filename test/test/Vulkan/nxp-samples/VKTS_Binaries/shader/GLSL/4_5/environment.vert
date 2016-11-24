#version 450 core

layout (binding = 0, std140) uniform _u_bufferViewProjection {
        mat4 projectionMatrix;
        mat4 viewMatrix;
} u_bufferViewProjection;

layout (binding = 1, std140) uniform _u_bufferTransform {
        mat4 modelMatrix;
        mat3 normalMatrix;
} u_bufferTransform;

layout (location = 0) in vec4 a_vertex;

layout (location = 0) out vec3 v_ray;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
	v_ray = normalize(vec3(a_vertex));
	
	gl_Position = u_bufferViewProjection.projectionMatrix * u_bufferViewProjection.viewMatrix * u_bufferTransform.modelMatrix * a_vertex;
}
