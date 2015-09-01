
precision lowp float;

varying float glass_a;
varying vec2 uv;
varying vec3 normal;
varying vec3 reflection;
varying vec3 refraction;

uniform vec3 light_unif;

uniform samplerCube env_tex_unif;
uniform sampler2D content_tex_unif;

void main()
{
//   float diffc = 0.2 * clamp(dot(normal, light_unif), 0.0, 1.0);

   float specc = 0.3 * pow(clamp(dot(normalize(reflection),
                                     light_unif),
                                 0.0,
                                 1.0),
                           8.0);

   vec4 reflc = 0.2 * textureCube(env_tex_unif, reflection);
   vec4 refrc = 1.0 * textureCube(env_tex_unif, refraction);

   vec2 bend = normal.xy * normal.xy * 0.1;

   vec4 raw_contentc = texture2D(content_tex_unif, uv + bend);

   float c_a = raw_contentc.a;

   vec4 contentc = 0.8 * raw_contentc * c_a;

   vec4 col =
      vec4(specc, specc, specc, 1.0) +
      reflc +
      (refrc * (1.0 - c_a)) +
      contentc;

   col.a = col.a * glass_a;

   gl_FragColor = col;
}
