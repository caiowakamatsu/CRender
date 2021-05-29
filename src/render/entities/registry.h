#pragma once

#include <entt/entt.hpp>

#include <render/entities/entity_type.h>
#include <render/entities/components.h>
#include <render/camera.h>
#include <objects/model.h>
#include <util/model_loader.h>

namespace cr
{
    class registry
    {
    public:
        registry();

        /* Load a model into the register after loading it in */
        void register_model(const cr::model_loader::model_data &data);

        void register_light(cr::entity::type light_type, const glm::vec3 &colour, const std::string &name);

        [[nodiscard]] cr::camera *camera();

        entt::basic_registry<uint32_t> entities;

    private:
        uint64_t _camera_entity;
    };
}    // namespace cr
