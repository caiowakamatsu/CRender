#version 440

layout (local_size_x = 8, local_size_y = 8) in;
layout (binding = 0, rgba8) uniform image2D img_output;
layout (binding = 0) uniform sampler2D rendered_scene;

#define TARGET_PIXEL gl_GlobalInvocationID

uniform float zoom;

uniform vec2 translation;
uniform ivec2 target_size;
uniform ivec2 scene_size;

void main ()
{
    vec2 translated_target_pixel = vec2(TARGET_PIXEL.xy) + translation;
    ivec2 img_size = target_size;
    vec2 target = (translated_target_pixel * zoom) / vec2(scene_size);

//    imageStore(img_output, ivec2(TARGET_PIXEL), vec4(1, 0));

    if (0 <= target.x && target.x <= 1 && 0 <= target.y && target.y <= 1)
        imageStore(img_output, ivec2(TARGET_PIXEL), vec4(texture(rendered_scene, target).xyz, 1.0f));

}
