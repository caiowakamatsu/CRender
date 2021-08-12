#version 450 core

out vec4 FragColor;

uniform vec3 camera_pos;
uniform vec3 colour;

in vec3 out_normal;
in vec3 worldspace_pos;
in vec2 uv;

uniform sampler2D mesh_texture;

void main()
{
//    FragColor = vec4(vec3(0.0f), 1.0f);
    if (colour.x < 0)
        FragColor = texture(mesh_texture, uv);
    else
        FragColor = vec4(colour, 1.0f);
}