#include "registry.h"

cr::registry::registry()
{
    entities.prepare<cr::entity::transforms>();
    entities.prepare<cr::entity::model_materials>();
    entities.prepare<cr::entity::model_geometry>();
    entities.prepare<cr::entity::light::point>();
    entities.prepare<cr::entity::light::directional>();
    entities.prepare<cr::entity::light::area>();

    // Create the camera
    _camera_entity = entities.create();

    entities.emplace<cr::entity::type>(_camera_entity, cr::entity::type::CAMERA);
    entities.emplace<cr::camera>(_camera_entity);
    entities.emplace<std::string>(_camera_entity, "Camera");
}

void cr::registry::register_light(cr::entity::type light_type, const glm::vec3 &colour, const std::string &name)
{
    const auto light = entities.create();

    entities.emplace<cr::entity::type>(light, light_type);
    entities.emplace<std::string>(light, name);

    switch (light_type)
    {
    case entity::type::POINT_LIGHT:
        entities.emplace<cr::entity::light::point>(light, 1.0f, colour, glm::vec3(0, 0, 0));
        break;
    case entity::type::DIRECTIONAL_LIGHT:
        entities.emplace<cr::entity::light::directional>(light, colour, glm::vec3(0, -1, 0), 1.0f);
        break;
    case entity::type::AREA_LIGHT:
        entities.emplace<cr::entity::light::area>(light, colour, glm::vec3(0, 0, 0), glm::vec2(1, 1), 1.0f);
        break;
    default:
        cr::exit("Light enum is not valid!");
    }
}

void cr::registry::register_model(const cr::model_loader::model_data &data)
{
    // Create the model embree instance
    auto        model_instance = cr::model::instance_geometry(data.vertices, data.vertex_indices);
    static auto current_model_count = uint32_t(0);

    auto entity = entities.create();

    entities.emplace<cr::entity::type>(entity, cr::entity::type::MODEL);

    entities.emplace<cr::entity::transforms>(entity);
    entities.get<cr::entity::transforms>(entity).data.emplace_back(glm::mat4(1));

    entities.emplace<cr::entity::model_materials>(entity, data.materials, data.material_indices);
    entities.emplace<cr::entity::model_geometry>(
      entity,
      model_instance.device,
      model_instance.scene,
      model_instance.geometry);
    entities.emplace<std::string>(
      entity,
      std::string("Model - " + std::to_string(++current_model_count)));
}

cr::camera *cr::registry::camera()
{
    return &entities.get<cr::camera>(_camera_entity);
}
