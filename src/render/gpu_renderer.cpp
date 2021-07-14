#include "gpu_renderer.h"

cr::gpu_renderer::gpu_renderer(cr::scene *scene, const glm::ivec2 &resolution)
    : _scene(scene), _resolution(resolution)
{
    // Compile compute shader

}

void cr::gpu_renderer::render_to(const GLuint target_texture, const glm::ivec2 &resolution) const
{
    // Todo: Update resolution and reload resolution dependent resources
    if (resolution != _resolution);

    // Bind path tracing compute shader

}
