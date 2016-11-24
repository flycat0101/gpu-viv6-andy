#version 450 core

layout (location = 0) in vec4 v_f_position;
layout (location = 1) in vec3 v_f_normal;
layout (location = 4) in vec2 v_f_texCoord;


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
    
    
    
    // UV map start

    // Out
    vec3 UV_Map_UV = vec3(v_f_texCoord, 0.0);
    
    // UV map end
    
    // Checker texture start

    // In
    vec3 Vector_0 = UV_Map_UV;
    vec4 Color1_1 = vec4(0.800, 0.800, 0.800, 1.000);
    vec4 Color2_2 = vec4(0.000, 0.000, 0.000, 1.000);
    float Scale_0 = 2.000;

    bool TempBool_0 = mod(floor(Vector_0.s * Scale_0), 2.0) == 1.0;
    bool TempBool_1 = mod(floor(Vector_0.t * Scale_0), 2.0) == 1.0;
    
    // Out
    vec4 Checker_Texture_Color = Color2_2;
    float Checker_Texture_Fac = 0.0;
        
    if ((TempBool_0 && !TempBool_1) || (!TempBool_0 && TempBool_1))
    {
        Checker_Texture_Color = Color1_1;
        Checker_Texture_Fac = 1.0;
    }
    
    // Checker texture end
    
    // Diffuse BSDF start

    // In
    vec4 Color_0 = vec4(0.800, 0.000, 0.000, 1.000);
    float Roughness_0 = 0.000;
    vec3 Normal_0 = normal;
    
    // Out
    vec4 Diffuse_BSDF_DiffuseNormalRoughness = vec4(Normal_0, Roughness_0);
    vec4 Diffuse_BSDF_DiffuseColor = Color_0;
    
    // Diffuse BSDF end
    
    // Mix shader start
    
    // In
    float Fac_0 = Checker_Texture_Fac;
    
    if (Fac_0 > 0.0)
    {
        discard;
    }

    // Out
    vec4 Mix_Shader_GlossyNormalRoughness = vec4(0.0, 0.0, 0.0, 0.0) * Fac_0;
    vec4 Mix_Shader_GlossyColor = vec4(0.0, 0.0, 0.0, 0.0) * Fac_0;

    vec4 Mix_Shader_DiffuseNormalRoughness = Diffuse_BSDF_DiffuseNormalRoughness * (1.0 - Fac_0);
    vec4 Mix_Shader_DiffuseColor = Diffuse_BSDF_DiffuseColor * (1.0 - Fac_0);

    // Mix shader end
    
    // Material start

    // Out
    ob_glossyNormalRoughness = vec4(Mix_Shader_GlossyNormalRoughness.xyz * 0.5 + 0.5, Mix_Shader_GlossyNormalRoughness.w);
    ob_glossyColor = Mix_Shader_GlossyColor;

    ob_diffuseNormalRoughness = vec4(Mix_Shader_DiffuseNormalRoughness.xyz * 0.5 + 0.5, Mix_Shader_DiffuseNormalRoughness.w);
    ob_diffuseColor = Mix_Shader_DiffuseColor;
    
    // Material end
}
