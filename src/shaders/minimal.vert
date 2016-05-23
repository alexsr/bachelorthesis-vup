#version 330 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec4 offset;

uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = proj * view * (position + offset);
}
