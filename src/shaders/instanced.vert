#version 330 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec4 offset;
layout (location = 2) in vec4 color;

uniform mat4 view;
uniform mat4 proj;

out vec4 pColor;

void main()
{
    pColor = color;
    vec4 pPos = proj * view * (position + offset);
    gl_Position = pPos;
}
