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
    _entities.register_model(model);
}

void cr::scene::set_skybox(cr::image &&skybox)
{
    if (!_skybox.has_value())
    {
        _skybox_texture = 0;
        glGenTextures(1, &_skybox_texture.value());
        glBindTexture(GL_TEXTURE_2D, _skybox_texture.value());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glBindTexture(GL_TEXTURE_2D, _skybox_texture.value());
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA8,
      skybox.width(),
      skybox.height(),
      0,
      GL_RGBA,
      GL_FLOAT,
      skybox.data());
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
        return glm::vec3(0.0);
    }
}

cr::ray::intersection_record cr::scene::cast_ray(const cr::ray ray)
{
    auto intersection = cr::ray::intersection_record();

    const auto &view =
      _entities.entities
        .view<cr::entity::instances, cr::entity::embree_ctx, cr::entity::model_materials>();

    for (const auto &entity : view)
    {
        const auto &instances  = _entities.entities.get<cr::entity::instances>(entity);
        const auto &embree_ctx = _entities.entities.get<cr::entity::embree_ctx>(entity);
        const auto &materials  = _entities.entities.get<cr::entity::model_materials>(entity);

        const auto current = cr::model::intersect(ray, instances, embree_ctx, materials);
        if (current.distance < intersection.distance) intersection = current;
    }

    return intersection;
}

cr::registry *cr::scene::registry()
{
    return &_entities;
}

std::optional<GLuint> cr::scene::skybox_handle() const noexcept
{
    return _skybox_texture;
}

glm::vec2 cr::scene::skybox_rotation() const noexcept
{
    return _skybox_rotation;
}

void cr::scene::set_sun_enabled(bool value)
{
    _sun_enabled = value;
}

bool cr::scene::is_sun_enabled() const noexcept
{
    return _sun_enabled;
}
