#pragma once

#include <vector>
#include <embree3/rtcore.h>
#include <glm/glm.hpp>

#include <render/ray.h>
#include <render/material/material.h>
#include <render/light.h>
#include <objects/model.h>
#include <util/exception.h>

namespace cr
{
    // Temporary name whilst I refactor
    class new_scene
    {
    public:
        new_scene() = default;

        void add_model(const cr::model& model);

        [[nodiscard]] cr::ray::intersection_record cast_ray(const cr::ray ray);

    private:
        struct
        {
            std::vector<cr::point_light>       point;
            std::vector<cr::directional_light> directional;
        } _lights;

        std::vector<cr::model> _models;
    };

    class scene
    {
    public:
        typedef uint32_t material_id;

        scene();

        scene(const scene &rhs);

        void add_triangles(
          const std::vector<glm::vec3> &vertices,
          const std::vector<uint64_t> & indices);

        void add_materials(
          const std::vector<cr::material> &materials,
          const std::vector<uint32_t> &    material_indices);

        struct intersection_record
        {
#define FLT_INFINITY std::numeric_limits<float>::infinity()
            float               distance = FLT_INFINITY;
            const cr::material *material = nullptr;
            glm::vec3           normal   = glm::vec3(FLT_INFINITY, FLT_INFINITY, FLT_INFINITY);
            glm::vec3 intersection_point = glm::vec3(FLT_INFINITY, FLT_INFINITY, FLT_INFINITY);
#undef FLT_INFINITY
        };
        [[nodiscard]] intersection_record cast_ray(const cr::ray &ray) const noexcept;

        [[nodiscard]] glm::vec3 sample_lights_at(const glm::vec3 &point) const noexcept;

        [[nodiscard]] std::vector<cr::material> &loaded_materials() noexcept;

        void update_materials(const std::vector<cr::material> &materials);

        std::vector<cr::point_light>       _point_lights;
        std::vector<cr::directional_light> _directional_lights;

    private:
        void _rebuild_scene();

        [[nodiscard]] bool _ray_occluded(const cr::ray &ray) const noexcept;

        std::vector<cr::material> _materials {};
        std::vector<uint32_t>     _material_indices {};

        std::vector<glm::vec3> _vertices {};
        std::vector<uint32_t>  _indices {};

        RTCScene    _scene;
        RTCDevice   _device;
        RTCGeometry _geometry;
    };
}    // namespace cr
