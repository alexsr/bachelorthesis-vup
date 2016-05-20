#version 330 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec3 offset;

uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = proj * view * vec4(position.xyz + offset, 1.0f);
}
