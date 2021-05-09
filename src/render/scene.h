#pragma once

#include <vector>
#include <random>

#include <embree3/rtcore.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <render/ray.h>
#include <render/material/material.h>
#include <render/light.h>
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

        [[nodiscard]] glm::vec3 sample_lights(const glm::vec3 &point, const cr::ray &ray, const cr::ray::intersection_record &record);

        [[nodiscard]] cr::registry* registry();

    private:
        [[nodiscard]] float _occluded(const glm::vec3 &origin, const glm::vec3 &target);

        [[nodiscard]] glm::vec3 _sample_lights_metal(const glm::vec3 &point, const cr::ray &ray, const cr::ray::intersection_record &record);

        [[nodiscard]] glm::vec3 _sample_lights_smooth(const glm::vec3 &point, const glm::vec3 &normal);

        cr::registry _entities;
    };
}    // namespace cr
