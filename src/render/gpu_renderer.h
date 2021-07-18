#pragma once

#include <stack>

#include <embree3/rtcore.h>

#include <render/scene.h>
#include <render/bvh_node.h>

#include <util/opengl.h>

namespace cr
{
    class gpu_renderer
    {
    public:
        gpu_renderer(cr::scene *scene, const glm::ivec2 &resolution);

        void build();

        void render(const glm::ivec2 &resolution) const;

        [[nodiscard]] GLuint texture() const;

    private:
        struct rendering_resources
        {
            glm::ivec4 resolution;
            glm::mat4 mvp;
            glm::vec3 camera_data;
        };

        struct
        {
            GLuint texture;
            GLuint shader;
            GLuint compute;

            GLuint render_data_buffer;
            GLuint bvh_data_buffer = ~0;
        } _opengl_handles;

        struct
        {
            RTCDevice device;
            RTCBVH    bvh;
        } _embree_ctx;

        void _update_resolution();

        void _build_bvh();

        glm::ivec2 _resolution;

        cr::scene *_scene;
    };
}    // namespace cr
