#version 450 core

out vec4 FragColor;

void main()
{
//    FragColor = (vec4(gl_FragCoord.z, 1.0f) + vec4(0.2, 0.2, 0.6, 1.0)) / 2.f;
//    FragColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    FragColor = vec4(gl_FragCoord.z);
}