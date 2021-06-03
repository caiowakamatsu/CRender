#include "registry.h"

cr::registry::registry()
{
    entities.prepare<cr::entity::transforms>();
    entities.prepare<cr::entity::model_materials>();
    entities.prepare<cr::entity::model_texcoords>();
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

void cr::registry::register_model(const cr::model_loader::model_data &data)
{
    auto persistent_texture_coords = std::make_unique<std::vector<glm::vec2>>();
    persistent_texture_coords->reserve(data.texture_coords.size());
    for (const auto &coord : data.texture_coords)
        persistent_texture_coords->push_back(coord);

    auto persistent_texture_indices = std::make_unique<std::vector<uint32_t>>();
    persistent_texture_indices->reserve(data.texture_indices.size());
    for (const auto index : data.texture_indices)
        persistent_texture_indices->push_back(index);

    // Create the model embree instance
    auto        model_instance = cr::model::instance_geometry(data.vertices, data.vertex_indices, *persistent_texture_coords, *persistent_texture_indices);
    static auto current_model_count = uint32_t(0);

    auto entity = entities.create();

    entities.emplace<cr::entity::type>(entity, cr::entity::type::MODEL);

    entities.emplace<cr::entity::transforms>(entity);
    entities.get<cr::entity::transforms>(entity).data.emplace_back(glm::mat4(1));

    entities.emplace<cr::entity::model_materials>(entity, data.materials, data.material_indices);
    entities.emplace<cr::entity::model_texcoords>(entity, std::move(persistent_texture_coords), std::move(persistent_texture_indices));
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
