
precision lowp float;

attribute vec3 pos_attr;
attribute vec3 norm_attr;

uniform mat4 proj_unif;
uniform mat4 modelview_unif;
uniform mat4 env_unif;

varying vec3 reflection;
varying vec3 refraction;
varying vec3 normal;
varying vec2 uv;
varying float glass_a;


void main()
{
   vec4 pos = modelview_unif * vec4(pos_attr, 1.0);
   gl_Position = proj_unif * pos;

   uv = gl_Position.xy / gl_Position.w * 0.5 + 0.5;

   normal = normalize(modelview_unif * vec4(norm_attr, 0.0)).xyz;

   float m = length(cross(normalize(pos.xyz),normal));
   glass_a = 1.0 - (m * m);

   reflection = (env_unif * vec4(reflect(pos.xyz, normal.xyz), 0.0)).xyz;

   refraction = (env_unif * vec4(refract(pos.xyz, normal.xyz, 0.8), 0.0)).xyz;
}
