#version 330 core
out vec4 FragColor;

uniform vec3 color;

void main()
{
    // Color uniforme con alfa fijo en 1.0
    FragColor = vec4(color, 1.0);
}
