#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(pos, 1.0);
}