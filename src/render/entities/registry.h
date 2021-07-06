#pragma once

#include <entt/entt.hpp>

#include <render/entities/entity_type.h>
#include <render/entities/components.h>
#include <render/camera.h>
#include <render/mesh.h>
#include <objects/model.h>
#include <util/asset_loader.h>
#include <util/sampling.h>
#include <variant>

namespace cr
{
    struct raster_objects
    {
        std::vector<cr::mesh> meshes;
    };

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
            uint32_t          entity_handle;
            cr::raster_objects meshes;
        };
        [[nodiscard]] registered_model register_model(const cr::asset_loader::model_data &data);

    private:
        [[nodiscard]] cr::raster_objects
          _get_meshes_by_material(const cr::asset_loader::model_data &data);

        [[nodiscard]] cr::raster_objects
          _upload_temporary_meshes(const std::vector<cr::temporary_mesh> &meshes);

        [[nodiscard]] std::vector<float> _zip_mesh_data(const cr::temporary_mesh &mesh);

        uint64_t _camera_entity;

        cr::entity::sun _sun {};
        glm::mat3 _sun_transform;
    };
}    // namespace cr
