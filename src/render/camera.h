#pragma once

#include <glm/glm.hpp>

#include <render/ray.h>

namespace cr
{
    class camera
    {
    public:
        explicit camera(glm::vec3 position = glm::vec3(3, 2, 2), glm::vec3 look_at = glm::vec3(0, 0, 0), float fov = 75, float aspect = 1);

        [[nodiscard]] cr::ray get_ray(float x, float y) const noexcept;

    private:
        glm::vec3 horizontal {};
        glm::vec3 vertical {};
        glm::vec3 position {};
        glm::vec3 center {};
    };
}    // namespace cr
