#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 normal;

uniform mat4 mvp;

out vec3 out_normal;
out vec3 worldspace_pos;
out vec2 uv;

void main()
{
    gl_Position = mvp * vec4(pos, 1.0);
    out_normal = normalize(normal);
    worldspace_pos = pos;
    uv = in_uv;
}