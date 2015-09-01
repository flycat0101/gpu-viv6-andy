attribute vec4 my_Vertex;
attribute vec2 my_Texcoor;
varying   vec2 vTexcoor;
uniform   mat4 my_TransformMatrix;

void main()
{
    vTexcoor = my_Texcoor;
    gl_Position = my_TransformMatrix * my_Vertex;
}
