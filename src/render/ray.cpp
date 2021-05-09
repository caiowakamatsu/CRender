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

cr::ray cr::ray::transform(const glm::mat4 &matrix) const noexcept
{
    const auto transformed_origin = matrix * glm::vec4(origin, 1.0f);
    const auto transformed_direction = matrix * glm::vec4(direction, 0.0f);

    return cr::ray(transformed_origin, glm::normalize(transformed_direction));
}
