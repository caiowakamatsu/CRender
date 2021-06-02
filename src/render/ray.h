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
            glm::vec2           uv;
            float               distance = std::numeric_limits<float>::infinity();
            const cr::material *material = nullptr;
            glm::vec3           normal;
            glm::vec3           intersection_point;
        };

        glm::vec3 origin;
        glm::vec3 direction;

        ray() = default;

        ray(const glm::vec3 &origin, const glm::vec3 &direction);

        [[nodiscard]] glm::vec3 inv_dir() const noexcept;

        [[nodiscard]] glm::vec3 at(float t) const noexcept;

        [[nodiscard]] cr::ray transform(const glm::mat4 &matrix) const noexcept;
    };
}    // namespace cr
