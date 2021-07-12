#pragma once

#include <entt/entt.hpp>

#include <render/entities/components.h>
#include <render/camera.h>
#include <objects/model.h>
#include <util/asset_loader.h>
#include <util/sampling.h>
#include <glad/glad.h>
#include <variant>

namespace cr
{
    class registry
    {
    public:
        registry();

        void set_sun(const cr::entity::sun &sun);

        [[nodiscard]] cr::camera *camera();

        [[nodiscard]] cr::entity::sun sun();

        [[nodiscard]] glm::mat3 sun_transform();

        entt::basic_registry<uint32_t> entities;

        /* Load a model into the register after loading it in */
        struct registered_model
        {
            uint32_t           entity_handle;
        };
        void register_model(const cr::asset_loader::model_data &data);

    private:
        void _upload_gpu_meshes(const cr::asset_loader::model_data &data, uint32_t entity);

        [[nodiscard]] std::vector<float> _zip_mesh_data(
          const std::vector<glm::vec3> &vertices,
          const std::vector<glm::vec3> &normals,
          const std::vector<glm::vec2> &texture_coords);

        uint64_t _camera_entity;

        cr::entity::sun _sun {};
        glm::mat3       _sun_transform;
    };
}    // namespace cr
