#include "scene.h"

namespace
{
    [[nodiscard]] float randf() noexcept
    {
        thread_local std::mt19937                          gen;
        thread_local std::uniform_real_distribution<float> dist(0.f, 1.f);
        return dist(gen);
    }
}    // namespace

void cr::scene::add_model(const cr::model_loader::model_data &model)
{
    _entities.register_model(model);
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

glm::vec3 cr::scene::sample_lights(const glm::vec3 &point, const glm::vec3 normal)
{
    auto current = glm::vec3(0.f, 0.f, 0.f);

    // Iterate through point lights
    for (const auto &entity : _entities.entities.view<cr::entity::light::point>())
    {
        const auto &light             = _entities.entities.get<cr::entity::light::point>(entity);
        const auto  distance_to_light = glm::distance(light.position, point);
        const auto  distance          = _occluded(point, light.position);

        if (distance_to_light < distance)
            current += (light.colour * light.intensity) / (distance_to_light * distance_to_light);
    }

    // Iterate through directional lights
    for (const auto &entity : _entities.entities.view<cr::entity::light::directional>())
    {
        const auto &light    = _entities.entities.get<cr::entity::light::directional>(entity);
        const auto  distance = _occluded(point, point - light.direction);

        if (distance == std::numeric_limits<float>::infinity())
            current +=
              (light.colour * light.intensity) * glm::max(0.f, glm::dot(normal, -light.direction));
    }

    // Iterate through area lights
    for (const auto &entity : _entities.entities.view<cr::entity::light::area>())
    {
        // Sample area light
        const auto &light = _entities.entities.get<cr::entity::light::area>(entity);
        const auto  sample_point =
          glm::vec3(light.matrix * glm::vec4((::randf() - .5) * 2.f, 0, (::randf() * .5) * 2.f, 1));

        const auto distance            = _occluded(point, sample_point);
        const auto inv_normal_to_light = glm::normalize(sample_point - point);
        const auto distance_to_light   = glm::distance(point, sample_point);

        if (distance_to_light < distance)
            current += (light.colour * light.intensity) *
              glm::max(0.f, glm::dot(normal, inv_normal_to_light));
    }

    const auto view = _entities.entities.view<cr::entity::type>();

    // Maybe use the thread pool for this looping in the future?
    for (const auto &entity : view)
    {
        const auto &type = _entities.entities.get<cr::entity::type>(entity);

        if (type == entity::type::POINT_LIGHT) { }
        else if (type == entity::type::DIRECTIONAL_LIGHT)
        {
        }
        else if (type == entity::type::AREA_LIGHT)
        {
        }
    }

    return current;
}

cr::registry *cr::scene::registry()
{
    return &_entities;
}

float cr::scene::_occluded(const glm::vec3 &origin, const glm::vec3 &target)
{
    const auto ray              = cr::ray(origin, glm::normalize(target - origin));
    const auto ray_intersection = cast_ray(ray);
    return cr::ray::intersection_record().distance;
}
