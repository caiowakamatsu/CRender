#include "scene.h"

namespace
{
    [[nodiscard]] float randf() noexcept
    {
        thread_local std::mt19937                          gen;
        thread_local std::uniform_real_distribution<float> dist(0.f, 1.f);
        return dist(gen);
    }

    [[nodiscard]] float intersect_unit_rect(const cr::ray &ray)
    {
        auto den    = glm::dot(ray.direction, glm::vec3(0, 1, 0));
        auto normal = glm::vec3(0, 1, 0);
        if (den < 0)
        {
            den    = glm::dot(ray.direction, glm::vec3(0, -1, 0));
            normal = glm::vec3(0, -1, 0);
        }

        if (den > 0.00001f)
        {
            const auto dist = glm::dot(-ray.origin, normal) / den;
            return dist;
        }
        return std::numeric_limits<float>::infinity();
    }
}    // namespace

void cr::scene::add_model(const cr::asset_loader::model_data &model)
{
    const auto  registered_model = _entities.register_model(model);
    const auto &mesh             = registered_model.meshes;
    auto        mesh_index       = scene::mesh_index();
    mesh_index.object_name       = model.name;
    mesh_index.index_start       = _meshes.size();
    mesh_index.index_end         = _meshes.size() + mesh.meshes.size();
    mesh_index.entity_handle     = registered_model.entity_handle;
    _models.push_back(std::move(mesh_index));

    for (const auto inner_mesh : mesh.meshes) _meshes.push_back(inner_mesh);
}

void cr::scene::set_skybox(cr::image &&skybox)
{
    _skybox = skybox;
}

void cr::scene::set_skybox_rotation(const glm::vec2 &rotation)
{
    _skybox_rotation = rotation;
}

glm::vec3 cr::scene::sample_skybox(float x, float y) const noexcept
{
    if (_skybox.has_value())
    {
        return _skybox->get_uv(x + _skybox_rotation.x, y + _skybox_rotation.y);
    }
    else
    {
        return glm::vec3();
    }
}

cr::ray::intersection_record cr::scene::cast_ray(const cr::ray ray)
{
    auto intersection = cr::ray::intersection_record();

    const auto &view =
      _entities.entities
        .view<cr::entity::transforms, cr::entity::model_geometry, cr::entity::model_materials>();

    for (const auto &entity : view)
    {
        const auto &transform = _entities.entities.get<cr::entity::transforms>(entity);
        const auto &geometry  = _entities.entities.get<cr::entity::model_geometry>(entity);
        const auto &materials = _entities.entities.get<cr::entity::model_materials>(entity);

        const auto current = cr::model::intersect(ray, transform, geometry, materials);
        if (current.distance < intersection.distance) intersection = current;
    }

    return intersection;
}

const std::vector<cr::mesh> &cr::scene::meshes() const noexcept
{
    return _meshes;
}

cr::registry *cr::scene::registry()
{
    return &_entities;
}

const std::vector<cr::scene::mesh_index> &cr::scene::models() const noexcept
{
    return _models;
}
std::vector<cr::mesh> &cr::scene::meshes()
{
    return _meshes;
}
