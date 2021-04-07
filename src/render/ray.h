#pragma once

#include <glm/glm.hpp>

#include <render/material/material.h>

namespace cr
{

    class ray
    {
    public:
        struct intersection_record
        {
            float         distance = std::numeric_limits<float>::infinity();
            cr::material *material = nullptr;
            glm::vec3     normal;
            glm::vec3     intersection_point;
        };

        glm::vec3 origin;
        glm::vec3 direction;

        ray() = default;

        ray(const glm::vec3 &origin, const glm::vec3 &direction);

        [[nodiscard]] glm::vec3 inv_dir() const noexcept;

        [[nodiscard]] glm::vec3 at(float t) const noexcept;
    };
}    // namespace cr
