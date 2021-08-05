#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <render/ray.h>
#include <util/numbers.h>

namespace cr
{
    class camera
    {
    public:
        enum class mode
        {
            perspective,
            orthographic,
        };

        camera(glm::vec3 position = glm::vec3(5, 500, 0), float fov = 75, mode camera_mode = mode::perspective);

        void translate(const glm::vec3 &translation);

        void rotate(const glm::vec3 &rotation);

        [[nodiscard]] glm::mat4 mat4() const noexcept;

        [[nodiscard]] cr::ray get_ray(float x, float y, float aspect);

        float fov;
        float scale;

        mode current_mode;
        glm::vec3 position {};
        glm::vec3 rotation {};
    private:

        void _update_cache();
        glm::mat4 _cached_matrix;
    };
}    // namespace cr
