
precision mediump float;

varying vec3 eye;

uniform samplerCube env_tex_unif;

void main()
{
   vec4 col =  textureCube(env_tex_unif, eye);

   gl_FragColor = col;
}
