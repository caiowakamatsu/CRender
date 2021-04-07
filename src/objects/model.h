#pragma once

#include <algorithm>
#include <vector>

#include <fmt/core.h>

#include <glm/glm.hpp>

#include <embree3/rtcore.h>

#include <render/material/material.h>
#include <render/ray.h>
#include <util/exception.h>

namespace cr
{
    class model
    {
    public:
        model(
          const std::vector<glm::vec3> &   vertices,
          const std::vector<cr::material> &materials,
          const std::vector<uint32_t>      vertex_indices,
          const std::vector<uint32_t>      material_indices);

        void add_transform(const glm::mat4 &transform);

        [[nodiscard]] cr::ray::intersection_record intersect(const cr::ray &ray);

    private:
        [[nodiscard]] cr::ray::intersection_record _intersect_self(const cr::ray &ray);

        std::vector<glm::mat4> _transforms;

        struct
        {
            std::vector<cr::material> data;
            std::vector<uint32_t>     indices;
        } _materials;

        RTCScene    _scene;
        RTCGeometry _geometry;
    };
}    // namespace cr
