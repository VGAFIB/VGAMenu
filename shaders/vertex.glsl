uniform vec3 color;

varying vec3 vColor;
varying vec2 pos;

void main(void)
{
    gl_Position = gl_Vertex;
    vColor = color;
    pos = gl_MultiTexCoord0;
}
