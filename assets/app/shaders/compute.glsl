#version 440

layout (local_size_x = 8, local_size_y = 8) in;
layout (binding = 0, rgba8) uniform image2D img_output;
layout (binding = 1) uniform sampler2D rendered_scene;

#define TARGET_PIXEL gl_GlobalInvocationID

uniform float zoom;

uniform vec2 translation;
uniform ivec2 target_size;
uniform ivec2 scene_size;

void main ()
{
    if (target_size.x > TARGET_PIXEL.x && target_size.y > TARGET_PIXEL.y)
    {
        vec2 translated_target_pixel = vec2(TARGET_PIXEL.xy) + translation;

        ivec2 img_size = target_size;
        vec2 target = (translated_target_pixel * zoom) / vec2(scene_size);

        bool oob = target.x < 0 || 1 < target.x || target.y < 0 || 1 < target.y;

        imageStore(img_output, ivec2(TARGET_PIXEL),
            oob ? vec4(0, 0, 0, 0) : texture(rendered_scene, target));
    }
}
