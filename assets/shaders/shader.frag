#version 450 core

out vec4 FragColor;

uniform vec3 camera_pos;

in vec3 out_normal;
in vec3 worldspace_pos;
in vec2 uv;

uniform sampler2D texture;

void main()
{

    // Calculate fake shading based on
    // VdotN
    // V: Normal from fragment to camera
    // N: Surface normal of fragment
//    vec3 view_normal = normalize(camera_pos - worldspace_pos);

//    float VdotN = clamp(dot(view_normal, out_normal), 0, 1);

//    FragColor = vec4(vec3(VdotN), 1.0f);
    FragColor = texture(texture, uv);
}