#version 440

layout (local_size_x = 1, local_size_y = 1) in;
layout (binding = 0     , rgba8)            uniform image2D img_output;
layout (binding = 1)                        uniform sampler2D rendered_scene;

#define TARGET_PIXEL gl_WorkGroupID
#define TARGET_SIZE imageSize(img_output)
#define SCENE_SIZE textureSize(rendered_scene, 0)

void main ()
{
    imageStore(img_output, ivec2(TARGET_PIXEL), vec4(0.25, 0.25, .6, 1));
}
