#pragma once

#include <stack>

#include <bvh/sweep_sah_builder.hpp>
#include <bvh/triangle.hpp>

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

        void render(const glm::ivec2 &resolution);

        [[nodiscard]] GLuint texture() const;

    private:
        void kernel_generate();

        void kernel_extend(std::uint32_t fired_rays);

        void kernel_shade(
          std::uint32_t intersections,
          std::uint32_t current_frame,
          std::uint32_t current_bounce);

        struct rendering_resources
        {
            [[nodiscard]] bool operator==(const rendering_resources &rhs) const noexcept
            {
                return rhs.mvp == mvp && rhs.camera_data == camera_data;
            }

            [[nodiscard]] bool operator!=(const rendering_resources &rhs) const noexcept
            {
                return !(*this == rhs);
            }

            glm::mat4 mvp;
            glm::vec4 camera_data;
        };

        struct gpu_material
        {
            glm::vec4 colour;
            glm::vec4 data1;
            glm::vec4 tex_type;
        };

        struct gpu_triangle
        {
            /*
             * v0 - X-Z = Vertex 0 XYZ, W = U
             * v1 - X-Z = Vertex 1 XYZ, W = V
             * v2 - X-Z = Vertex 2 XYZ, W = MaterialID (as int)
             */
            glm::vec4 v0;
            glm::vec4 v1;
            glm::vec4 v2;
        };

        static void flatten_nodes(
          std::vector<bvh_node> &               nodes,
          size_t                                flat_parent_idx,
          embree_node *                         parent,
          std::vector<gpu_triangle> &           new_prims,
          const std::vector<gpu_triangle> &prims);

        struct
        {
            GLuint generate;
            GLuint extend;
            GLuint shade;

            struct
            {
                GLuint generate;
                GLuint extend;
                GLuint shade;
            } shaders;
        } _kernels;

        struct
        {
            GLuint scene;
            GLuint ray;
            GLuint intersections;
            GLuint ray_count;
        } _buffers;

        struct
        {
            GLuint throughput;
            GLuint colour;
            GLuint final_image;

            GLuint bvh_data_buffer      = ~0;
            GLuint triangle_data_buffer = ~0;
            GLuint material_data_buffer = ~0;
        } _opengl_handles;

        struct
        {
            RTCDevice device;
            RTCBVH    bvh;
        } _embree_ctx;

        void _update_resolution();

        void _build_bvh();

        void _build_material_buffer();

        std::uint32_t _current_frame = 0;

        glm::ivec2 _resolution;

        rendering_resources _previous_resources;

        cr::scene *_scene;
    };
}    // namespace cr
