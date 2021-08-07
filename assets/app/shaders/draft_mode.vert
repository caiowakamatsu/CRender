#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 normal;

uniform mat4 mvp;
uniform vec3 camera_pos; // Used for determining if the normal is flipped around

out vec3 out_normal;
out vec3 worldspace_pos;
out vec2 uv;

void main()
{
    gl_Position = mvp * vec4(pos, 1.0);

    vec3 dir_to_cam = normalize(camera_pos - gl_Position.xyz);

    if (dot(dir_to_cam, normal) < 0)
        out_normal = -normal;
    else
        out_normal = normal;
    worldspace_pos = pos;
    uv = in_uv;
}