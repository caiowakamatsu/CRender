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
#include <objects/image.h>
#include <util/exception.h>

namespace cr
{
    class scene
    {
    public:
        scene() = default;

        void add_model(const cr::asset_loader::model_data &model);

        void set_skybox(cr::image &&skybox);

        void set_skybox_rotation(const glm::vec2 &rotation);

        void set_sun_enabled(bool value);

        [[nodiscard]] glm::vec3 sample_skybox(float x, float y) const noexcept;

        [[nodiscard]] cr::ray::intersection_record cast_ray(const cr::ray ray);

        [[nodiscard]] cr::registry *registry();

        [[nodiscard]] std::optional<GLuint> skybox_handle() const noexcept;

        [[nodiscard]] glm::vec2 skybox_rotation() const noexcept;

        [[nodiscard]] bool is_sun_enabled() const noexcept;

    private:
        bool _sun_enabled = true;

        std::optional<cr::image> _skybox;
        std::optional<GLuint>    _skybox_texture;

        glm::vec2 _skybox_rotation;

        cr::registry _entities;
    };
}    // namespace cr
