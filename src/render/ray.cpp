#include "ray.h"

cr::ray::ray(const glm::vec3 &origin, const glm::vec3 &direction)
    : origin(origin), direction(direction)
{
}

glm::vec3 cr::ray::inv_dir() const noexcept
{
    return 1.f / direction;
}

glm::vec3 cr::ray::at(float t) const noexcept
{
    return origin + direction * t;
}
