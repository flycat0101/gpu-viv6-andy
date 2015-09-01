
precision highp float;

uniform sampler2D my_Sampler;
varying vec2 vTexcoor;

void main (void)
{
    gl_FragColor = texture2D(my_Sampler, vTexcoor);
}
