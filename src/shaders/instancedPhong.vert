#version 330 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 offset;
layout (location = 3) in vec4 color;

uniform mat4 view;
uniform mat4 proj;

out vec3 pPos;
out vec3 pNormal;
out vec4 pColor;

void main()
{
    pColor = color;
    vec4 pos = view * (position + offset);
    pPos = pos.xyz;
    pNormal = normal;
    gl_Position = proj * pos;
}
