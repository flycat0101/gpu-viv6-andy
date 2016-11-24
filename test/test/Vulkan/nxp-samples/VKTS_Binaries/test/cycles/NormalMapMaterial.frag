#version 450 core

layout (location = 0) in vec4 v_f_position;
layout (location = 1) in vec3 v_f_normal;
layout (location = 2) in vec3 v_f_bitangent;
layout (location = 3) in vec3 v_f_tangent;
layout (location = 4) in vec2 v_f_texCoord;

layout (binding = 4) uniform sampler2D u_texture0;

layout (location = 4) out vec4 ob_position;                // Position as NDC. Last element used but could be freed.
layout (location = 3) out vec4 ob_glossyNormalRoughness;   // Glossy normal and roughness.
layout (location = 2) out vec4 ob_glossyColor;             // Glossy color and alpha.
layout (location = 1) out vec4 ob_diffuseNormalRoughness;  // Diffuse normal and roughness.
layout (location = 0) out vec4 ob_diffuseColor;            // Diffuse color and alpha.

mat4 translate(vec3 t)
{
    return mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, t.x, t.y, t.z, 1.0);
}

mat4 rotateRzRyRx(vec3 rotate)
{
    if (rotate.x == 0.0 && rotate.y == 0.0 && rotate.z == 0.0)
    {
        return mat4(1.0);
    }

    float rz = radians(rotate.z);
    float ry = radians(rotate.y);
    float rx = radians(rotate.x);
    float sx = sin(rx);
    float cx = cos(rx);
    float sy = sin(ry);
    float cy = cos(ry);
    float sz = sin(rz);
    float cz = cos(rz);

    return mat4(cy * cz, cy * sz, -sy, 0.0, -cx * sz + cz * sx * sy, cx * cz + sx * sy * sz, cy * sx, 0.0, sz * sx + cx * cz * sy, -cz * sx + cx * sy * sz, cx * cy, 0.0, 0.0, 0.0, 0.0, 1.0);
}

mat4 scale(vec3 s)
{
    return mat4(s.x, 0.0, 0.0, 0.0, 0.0, s.y, 0.0, 0.0, 0.0, 0.0, s.z, 0.0, 0.0, 0.0, 0.0, 1.0);
}

void main()
{
    vec4 position = v_f_position / v_f_position.w; 
    
    ob_position = position * 0.5 + 0.5;
    
    vec3 normal = normalize(v_f_normal);
    vec3 bitangent = normalize(v_f_bitangent);
    vec3 tangent = normalize(v_f_tangent);
    
    mat3 objectToWorldMatrix = mat3(tangent, bitangent, normal);
    
    
    // UV map start

    // Out
    vec3 UV_Map_UV = vec3(v_f_texCoord, 0.0);
    
    // UV map end
    
    // Image texture start

    // In
    vec3 Vector_0 = UV_Map_UV;
    
    // Out
    vec4 Image_Texture_Color = texture(u_texture0, Vector_0.st).rgba;
    float Image_Texture_Alpha = texture(u_texture0, Vector_0.st).a;
    
    // Image texture end
    
    // Normal map start

    // In
    float Strength_0 = 1.000;
    vec3 Color_2 = objectToWorldMatrix * normalize(Image_Texture_Color.xyz * 2.0 - 1.0);
    
    // Out
    
    vec3 Normal_Map_Normal = mix(normal, Color_2, Strength_0);
    
    // Normal map end
    
    // Glossy BSDF start

    // In
    vec4 Color_1 = vec4(0.021, 0.800, 0.045, 1.000);
    float Roughness_1 = 0.200;
    vec3 Normal_1 = Normal_Map_Normal;
    
    // Out
    vec4 Glossy_BSDF_GlossyNormalRoughness = vec4(Normal_1, Roughness_1);
    vec4 Glossy_BSDF_GlossyColor = Color_1;
    
    // Glossy BSDF end
    
    // Diffuse BSDF start

    // In
    vec4 Color_0 = vec4(0.800, 0.000, 0.000, 1.000);
    float Roughness_0 = 0.100;
    vec3 Normal_0 = Normal_Map_Normal;
    
    // Out
    vec4 Diffuse_BSDF_DiffuseNormalRoughness = vec4(Normal_0, Roughness_0);
    vec4 Diffuse_BSDF_DiffuseColor = Color_0;
    
    // Diffuse BSDF end
    
    // Add shader start

    // Out
    vec4 Add_Shader_GlossyNormalRoughness = Glossy_BSDF_GlossyNormalRoughness;
    vec4 Add_Shader_GlossyColor = Glossy_BSDF_GlossyColor;

    vec4 Add_Shader_DiffuseNormalRoughness = Diffuse_BSDF_DiffuseNormalRoughness;
    vec4 Add_Shader_DiffuseColor = Diffuse_BSDF_DiffuseColor;
    
    // Add shader end
    
    // Material start

    // Out
    ob_glossyNormalRoughness = vec4(Add_Shader_GlossyNormalRoughness.xyz * 0.5 + 0.5, Add_Shader_GlossyNormalRoughness.w);
    ob_glossyColor = Add_Shader_GlossyColor;

    ob_diffuseNormalRoughness = vec4(Add_Shader_DiffuseNormalRoughness.xyz * 0.5 + 0.5, Add_Shader_DiffuseNormalRoughness.w);
    ob_diffuseColor = Add_Shader_DiffuseColor;
    
    // Material end
}
