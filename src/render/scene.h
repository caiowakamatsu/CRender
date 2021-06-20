#pragma once

#include <vector>
#include <random>

#include <embree3/rtcore.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <render/ray.h>
#include <render/mesh.h>
#include <render/material/material.h>
#include <render/entities/registry.h>
#include <objects/model.h>
#include <objects/image.h>
#include <util/exception.h>

namespace cr
{
    class scene
    {
    public:
        scene() = default;

        void add_model(const cr::model_loader::model_data &model);

        void set_skybox(cr::image &&skybox);

        [[nodiscard]] glm::vec3 sample_skybox(float x, float y) const noexcept;

        [[nodiscard]] cr::ray::intersection_record cast_ray(const cr::ray ray);

        [[nodiscard]] const std::vector<cr::mesh> &meshes() const noexcept;

        [[nodiscard]] cr::registry* registry();

    private:
        std::optional<cr::image> _skybox;

        std::vector<cr::mesh> _meshes;

        cr::registry _entities;
    };
}    // namespace cr
