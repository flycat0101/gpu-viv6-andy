#version 450 core

layout (location = 0) in vec4 v_f_position;
layout (location = 1) in vec3 v_f_normal;


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
    
    
    
    // Invert start

    // In
    float Fac_0 = 0.750;
    vec4 Color_1 = vec4(0.800, 0.000, 0.800, 1.000);
    
    // Out
    vec4 Invert_Color = mix(Color_1, vec4(1.0 - Color_1.r, 1.0 - Color_1.g, 1.0 - Color_1.b, 1.0 - Color_1.a), Fac_0);
    
    // Invert end
    
    // Diffuse BSDF start

    // In
    vec4 Color_0 = Invert_Color;
    float Roughness_0 = 0.200;
    vec3 Normal_0 = normal;
    
    // Out
    vec4 Diffuse_BSDF_DiffuseNormalRoughness = vec4(Normal_0, Roughness_0);
    vec4 Diffuse_BSDF_DiffuseColor = Color_0;
    
    // Diffuse BSDF end
    
    // Material start

    // Out
    ob_glossyNormalRoughness = vec4(vec4(0.0, 0.0, 0.0, 0.0).xyz * 0.5 + 0.5, vec4(0.0, 0.0, 0.0, 0.0).w);
    ob_glossyColor = vec4(0.0, 0.0, 0.0, 0.0);

    ob_diffuseNormalRoughness = vec4(Diffuse_BSDF_DiffuseNormalRoughness.xyz * 0.5 + 0.5, Diffuse_BSDF_DiffuseNormalRoughness.w);
    ob_diffuseColor = Diffuse_BSDF_DiffuseColor;
    
    // Material end
}
