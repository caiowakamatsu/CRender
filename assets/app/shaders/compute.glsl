#version 440

layout (local_size_x = 1, local_size_y = 1) in;
layout (binding = 0, rgba8)            uniform image2D img_output;
layout (binding = 1)                        uniform sampler2D rendered_scene;

#define TARGET_PIXEL gl_GlobalInvocationID
#define TARGET_SIZE imageSize(img_output)
#define SCENE_SIZE textureSize(rendered_scene, 0)

uniform float zoom;
uniform vec2  translation;

void main ()
{
    vec2 translated_target_pixel = vec2(TARGET_PIXEL.xy) + translation;
    vec2 target = translated_target_pixel * zoom;
//    imageStore(img_output, ivec2(TARGET_PIXEL), texture(rendered_scene, target / vec2(SCENE_SIZE.xy)));
    imageStore(img_output, ivec2(TARGET_PIXEL), vec4(1, 1, 1, 1));
}
