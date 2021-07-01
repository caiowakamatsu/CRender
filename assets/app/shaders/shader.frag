#version 450 core

out vec4 FragColor;

uniform vec3 camera_pos;

in vec3 out_normal;
in vec3 worldspace_pos;
in vec2 uv;

uniform sampler2D mesh_texture;

void main()
{
//    FragColor = vec4(vec3(0.0f), 1.0f);
    FragColor = texture(mesh_texture, uv);
}