#include "camera.h"

cr::camera::camera(const glm::vec3 position, const glm::vec3 look_at, float fov)
    : look_at(look_at), fov(fov)
{
    const auto theta       = fov * 3.141592f / 180.0f;
    const auto half_height = tanf(theta / 2.f);
    const auto half_width  = half_height;

    constexpr auto up = glm::vec3(0, 1, 0);
    const auto     w  = glm::normalize(look_at - position);
    const auto     u  = glm::normalize(glm::cross(up, w));
    const auto     v  = glm::cross(w, u);

    this->horizontal = u * half_width;
    this->vertical   = v * half_height;
    this->position   = position;
    this->center     = position + w;
}
cr::ray cr::camera::get_ray(float x, float y, const cr::camera &camera)
{
    const auto x_offset  = camera.horizontal * (x * 2.f - 1);
    const auto y_offset  = camera.vertical * (y * 2.f - 1);
    const auto direction = glm::normalize((camera.center + x_offset + y_offset) - camera.position);

    auto ray      = cr::ray();
    ray.origin    = camera.position;
    ray.direction = direction;

    return ray;
}
