
precision mediump float;

attribute vec3 pos_attr;
attribute vec2 uv_attr;

uniform mat4 proj_unif;
uniform mat4 modelview_unif;

varying vec2 uv;


void main()
{
    vec4 pos = modelview_unif * vec4(pos_attr, 1.0);
    gl_Position = proj_unif * pos;

    uv = uv_attr;
}
