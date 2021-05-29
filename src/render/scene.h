#pragma once

#include <vector>
#include <random>

#include <embree3/rtcore.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <render/ray.h>
#include <render/material/material.h>
#include <render/entities/registry.h>
#include <objects/model.h>
#include <util/exception.h>

namespace cr
{
    class scene
    {
    public:
        scene() = default;

        void add_model(const cr::model_loader::model_data &model);

        [[nodiscard]] cr::ray::intersection_record cast_ray(const cr::ray ray);

        [[nodiscard]] cr::registry* registry();

    private:
        cr::registry _entities;
    };
}    // namespace cr
