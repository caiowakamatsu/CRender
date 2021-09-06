#include "scene.h"

namespace
{
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

void cr::scene::add_model(const cr::asset_loader::loaded_model &model)
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
        return glm::vec3();
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

cr::scene::nee_sample cr::scene::sample_light(const cr::ray::intersection_record &record, cr::random *random)
{
    auto sample       = nee_sample();
    sample.intersected = false;

    const auto &view = _entities.entities.view<cr::entity::emissive_triangles>();

    auto sum = glm::vec3(0);

    for (const auto &entity : view)
    {
        const auto &instances  = _entities.entities.get<cr::entity::instances>(entity);
        const auto &embree_ctx = _entities.entities.get<cr::entity::embree_ctx>(entity);
        const auto &materials  = _entities.entities.get<cr::entity::model_materials>(entity);
        const auto &emissive   = _entities.entities.get<cr::entity::emissive_triangles>(entity);
        const auto &geometry   = _entities.entities.get<cr::entity::geometry>(entity);

        const auto tri_pdf = 1.0f / emissive.emissive_indices.size();

        auto tri = glm::min(static_cast<uint64_t>(random->next_float() * emissive.emissive_indices.size()), emissive.emissive_indices.size() - 1);

        const auto idx = emissive.emissive_indices[tri] * 3;
        const auto v0  = geometry.vert_coords->operator[](idx + 0);
        const auto v1  = geometry.vert_coords->operator[](idx + 1);
        const auto v2  = geometry.vert_coords->operator[](idx + 2);

        const auto [sample_point, normal, area_pdf] = cr::sampling::sample_triangle(v0, v1, v2, random);

        const auto sample_pdf = area_pdf * tri_pdf;

        // At this point we have everything we need
        auto ray = cr::ray(
          record.intersection_point + record.normal * cr::numbers<float>::elipson,
          glm::normalize(sample_point - record.intersection_point));


        const auto current = cr::model::intersect(ray, instances, embree_ctx, materials);
        if (current.prim_id == idx / 3)
        {
            // geometric term
            const auto sample_cosine = glm::max(0.0f, glm::dot(record.normal, ray.direction));

            const auto light_cosine = glm::abs(glm::dot(normal, ray.direction));
            const auto light_dist_2 = glm::dot(sample_point - record.intersection_point, sample_point - record.intersection_point);

            // There was an intersection, add the contribution
            sample.contribution = current.material->info.emission *
                                  glm::vec3(current.material->info.colour);
            sample.pdf = sample_pdf;
            sample.intersected = true;
            sample.distance = current.distance;
            sample.geometry_term = (sample_cosine * light_cosine) / light_dist_2;

        } else {
            sample.intersected = false;
        }
    }

    return sample;
}
