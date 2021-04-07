#include "light.h"
cr::point_light::point_light(const glm::vec3 &position, const glm::vec3 &colour, float strength)
    : position(position), colour(colour * strength)
{
}

glm::vec3 cr::point_light::sample_dir(const glm::vec3 &point) const noexcept
{
    return glm::normalize(position - point);
}

cr::directional_light::directional_light(
  const glm::vec3 &direction,
  const glm::vec3 &colour,
  float            strength)
    : direction(glm::normalize(direction)), colour(colour * strength)
{
}

glm::vec3 cr::directional_light::sample_dir(const glm::vec3 &point) const noexcept
{
    return -direction;
}
