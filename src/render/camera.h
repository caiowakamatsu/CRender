#pragma once

#include <glm/glm.hpp>

#include <render/ray.h>

namespace cr
{
    class camera
    {
    public:
        explicit camera(glm::vec3 position = glm::vec3(0, 2.5, 5), glm::vec3 look_at = glm::vec3(0, 2, 0), float fov = 75);

        [[nodiscard]] static cr::ray get_ray(float x, float y, const camera &camera);

        glm::vec3 position {};
        glm::vec3 look_at;
        float fov;
    private:
        glm::vec3 horizontal {};
        glm::vec3 vertical {};
        glm::vec3 center {};
    };
}    // namespace cr
