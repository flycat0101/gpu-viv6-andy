#version 450 core

#define BIAS 0.01

layout (binding = 2, std140) uniform _u_bufferFrag {
	vec3 light;
} u_bufferFrag;

layout (binding = 8) uniform sampler2D u_emissiveTexture;
layout (binding = 9) uniform sampler2D u_alphaTexture;
layout (binding = 10) uniform sampler2D u_normalTexture;
layout (binding = 11) uniform sampler2D u_ambientTexture;
layout (binding = 12) uniform sampler2D u_diffuseTexture;
layout (binding = 13) uniform sampler2D u_specularTexture;
layout (binding = 14) uniform sampler2D u_specularShininessTexture;

layout (binding = 4) uniform sampler2D u_shadowTexture;

layout (location = 0) in vec3 v_eye;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_texCoord;
layout (location = 3) in vec4 v_projCoord;

layout (location = 0) out vec4 ob_fragColor;

void main(void)
{
	// Ambient and emissive color.
	vec4 color = texture(u_ambientTexture, v_texCoord) + texture(u_emissiveTexture, v_texCoord);
	
	vec3 normal = normalize(v_normal);
	
	float nDotL = max(dot(u_bufferFrag.light, normal), 0.0);
	
	if (nDotL > 0.0)
	{
		// Diffuse color.
		color += texture(u_diffuseTexture, v_texCoord) * nDotL;
		
		if (v_projCoord.z <= texture(u_shadowTexture, v_projCoord.xy).r + BIAS)
		{
			vec3 eye = normalize(v_eye);
		
			// Incident vector is opposite light direction vector.
			vec3 reflection = reflect(-u_bufferFrag.light, normal);
			
			float eDotR = max(dot(eye, reflection), 0.0);
			
			if (eDotR > 0.0)
			{
				// Shininess is stored as 1.0/128.0.
				float shininess = texture(u_specularShininessTexture, v_texCoord).r * 128.0;
			
				// Specular color.
				color += texture(u_specularTexture, v_texCoord) * pow(eDotR, shininess);
			}
		}
		else
		{
			color *= 0.5;
		}
	}
	
	ob_fragColor = color;
}
