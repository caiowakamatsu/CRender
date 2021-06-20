#pragma once

#include <entt/entt.hpp>

#include <render/entities/entity_type.h>
#include <render/entities/components.h>
#include <render/camera.h>
#include <render/mesh.h>
#include <objects/model.h>
#include <util/asset_loader.h>
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

        [[nodiscard]] cr::camera *camera();

        entt::basic_registry<uint32_t> entities;

        /* Load a model into the register after loading it in */
        [[nodiscard]] cr::raster_objects register_model(const cr::asset_loader::model_data &data);

    private:

        [[nodiscard]] cr::raster_objects _get_meshes_by_material(const cr::asset_loader::model_data &data);

        [[nodiscard]] cr::raster_objects _upload_temporary_meshes(const std::vector<cr::temporary_mesh> &meshes);

        [[nodiscard]] std::vector<float> _zip_mesh_data(const cr::temporary_mesh &mesh);

        uint64_t _camera_entity;
    };
}    // namespace cr
