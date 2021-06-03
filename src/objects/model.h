#pragma once

#include <algorithm>
#include <vector>

#include <fmt/core.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <embree3/rtcore.h>

#include <render/material/material.h>
#include <render/ray.h>
#include <render/entities/components.h>
#include <util/model_loader.h>
#include <util/exception.h>

namespace cr
{
    namespace model
    {
        [[nodiscard]] cr::entity::model_geometry instance_geometry(
          const std::vector<glm::vec3> &vertices,
          const std::vector<uint32_t> & indices,
          const std::vector<glm::vec2> &tex_coords,
          const std::vector<uint32_t> &tex_indices);

        [[nodiscard]] cr::ray::intersection_record intersect(
          const cr::ray &                    ray,
          const cr::entity::transforms &     transforms,
          const cr::entity::model_geometry & geometry,
          const cr::entity::model_materials &u);

    }    // namespace model

}    // namespace cr
