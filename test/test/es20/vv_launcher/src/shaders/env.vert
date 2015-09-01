
precision mediump float;

attribute vec3 pos_attr;

uniform mat4 proj_unif;
uniform mat4 env_unif;

varying vec3 eye;


void main()
{
   vec4 pos = proj_unif * vec4(pos_attr, 1.0);
   gl_Position = pos;

   eye = (env_unif * vec4(pos_attr.xyz, 0.0)).xyz;
}
