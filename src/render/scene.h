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
        struct mesh_index
        {
            std::string object_name;
            uint64_t    index_start;
            uint64_t    index_end;
        };

        scene() = default;

        void add_model(const cr::asset_loader::model_data &model);

        void set_skybox(cr::image &&skybox);

        void set_skybox_rotation(const glm::vec2 &rotation);

        [[nodiscard]] glm::vec3 sample_skybox(float x, float y) const noexcept;

        [[nodiscard]] cr::ray::intersection_record cast_ray(const cr::ray ray);

        [[nodiscard]] const std::vector<cr::mesh> &meshes() const noexcept;

        [[nodiscard]] cr::registry *registry();

    private:
        std::optional<cr::image> _skybox;

        std::vector<cr::mesh> _meshes;

        std::vector<mesh_index> _models;

        glm::vec2 _skybox_rotation;

        cr::registry _entities;
    };
}    // namespace cr
