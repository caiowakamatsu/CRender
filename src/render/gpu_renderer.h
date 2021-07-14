#pragma once

#include <render/scene.h>

namespace cr
{
    class gpu_renderer
    {
    public:
        gpu_renderer(cr::scene *scene, const glm::ivec2 &resolution);

        void render_to(const GLuint target_texture, const glm::ivec2 &resolution) const;

    private:

        struct rendering_resources
        {
//            GLuint
        };

        glm::ivec2 _resolution;

        cr::scene *_scene;

    };
}
