#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Calcula la posici�n final del v�rtice en coordenadas de clip
    gl_Position = projection * view * model * vec4(position, 1.0);
}
