#version 450 core

#define VKTS_METALLIC 0.9

#define VKTS_NORMAL_VALID_BIAS 0.1

layout (binding = 9, std140) uniform _u_bufferMatrices {
        mat4 inverseProjectionMatrix;
        mat4 inverseViewMatrix;
} u_bufferMatrices;

layout (binding = 8) uniform sampler2D u_lut;
layout (binding = 7) uniform samplerCube u_specularCubemap;
layout (binding = 6) uniform samplerCube u_diffuseCubemap;

layout (binding = 5) uniform sampler2D u_depth;                   // Depth.
layout (binding = 4) uniform sampler2D u_position;                // Position as NDC. Last element used but could be freed.
layout (binding = 3) uniform sampler2D u_glossyNormalRoughness;   // Glossy normal and roughness.
layout (binding = 2) uniform sampler2D u_glossyColor;             // Glossy color and alpha.
layout (binding = 1) uniform sampler2D u_diffuseNormalRoughness;  // Diffuse normal and roughness.
layout (binding = 0) uniform sampler2D u_diffuseColor;            // Diffuse color and alpha.

layout (location = 0) in vec2 v_texCoord;

layout (location = 0) out vec4 ob_fragColor;

vec3 renderLambert(vec3 N)
{
    return texture(u_diffuseColor, v_texCoord).rgb * texture(u_diffuseCubemap, mat3(u_bufferMatrices.inverseViewMatrix) * N).rgb;
}

vec3 renderCookTorrance(vec3 N, vec3 V)
{
    vec3 noColor = vec3(0.0, 0.0, 0.0);

    // Note: reflect takes incident vector.
    // Note: Use N instead of H for approximation.
    vec3 L = reflect(-V, N);
    
    float NdotL = dot(N, L);
    float NdotV = dot(N, V);
    
    // Lighted and visible
    if (NdotL > 0.0 && NdotV >= 0.0)
    {
        int levels = textureQueryLevels(u_specularCubemap); 
    
        float roughness = texture(u_glossyNormalRoughness, v_texCoord).a;
    
        float scaledRoughness = roughness * float(levels);
        
        float rLow = floor(scaledRoughness);
        float rHigh = ceil(scaledRoughness);    
        float rFraction = scaledRoughness - rLow;
        
        vec3 prefilteredColor = mix(textureLod(u_specularCubemap, mat3(u_bufferMatrices.inverseViewMatrix) * L, rLow).rgb, textureLod(u_specularCubemap, mat3(u_bufferMatrices.inverseViewMatrix) * L, rHigh).rgb, rFraction);

        vec2 envBRDF = texture(u_lut, vec2(NdotV, roughness)).rg;
        
        return texture(u_glossyColor, v_texCoord).rgb * prefilteredColor * (VKTS_METALLIC * envBRDF.x + envBRDF.y);
    }
    
    return noColor;
}

void main(void)
{
    float depth = texture(u_depth, v_texCoord).r;
    
    if (depth == 1.0)
    {
        discard;
    }
    
    //
    
    vec3 color = vec3(0.0, 0.0, 0.0);
    
    int valid = 0;
    
    //
    // Diffuse
    //
    
    vec3 normal = texture(u_diffuseNormalRoughness, v_texCoord).rgb * 2.0 - 1.0;
    
    float normalLength = length(normal);
    
    if (normalLength > VKTS_NORMAL_VALID_BIAS)
    {
        vec3 N = normal / normalLength;
        
        // FIXME: Use for diffuseRoughness > 0.0 Oren-Nayar.
        //float diffuseRoughness = texture(u_diffuseNormalRoughness, v_texCoord).a;
        
        color += renderLambert(N);
        
        valid = 1;
    }

    //
    // Specular
    //

    normal = texture(u_glossyNormalRoughness, v_texCoord).rgb * 2.0 - 1.0;
    
    normalLength = length(normal);
    
    if (normalLength > VKTS_NORMAL_VALID_BIAS)
    {
        vec3 N = normal / normalLength;


        vec4 vertex = u_bufferMatrices.inverseProjectionMatrix * (texture(u_position, v_texCoord) * 2.0 - 1.0);
        
        vertex /= vertex.w;
        
        vec3 V = -normalize(vertex.xyz);


        color += renderCookTorrance(N, V);
        
        valid = 1;
    }
    
    if (valid == 0)
    {
        discard;
    }
    
    //

	ob_fragColor = vec4(color, 1.0);
}
