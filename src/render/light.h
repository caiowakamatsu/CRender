#pragma once

#include <glm/glm.hpp>

#include <render/ray.h>

namespace cr
{
    class point_light
    {
    public:
        point_light(const glm::vec3 &position, const glm::vec3 &colour, float strength = 1.f);

        [[nodiscard]] glm::vec3 sample_dir(const glm::vec3 &point) const noexcept;

        const glm::vec3 position;
        const glm::vec3 colour;
    };

    class directional_light
    {
    public:
        directional_light(
          const glm::vec3 &direction,
          const glm::vec3 &colour,
          float            strength = 1.f);

        [[nodiscard]] glm::vec3 sample_dir(const glm::vec3 &point) const noexcept;

        const glm::vec3 direction;
        const glm::vec3 colour;
    };

}    // namespace cr