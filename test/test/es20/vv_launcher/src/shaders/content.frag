
precision mediump float;

varying vec2 uv;

uniform sampler2D tex_unif;

void main()
{
   gl_FragColor = texture2D(tex_unif, uv);
}
