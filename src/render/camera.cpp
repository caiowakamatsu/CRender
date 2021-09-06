#include "camera.h"

cr::camera::camera(glm::vec3 position, float fov, mode camera_mode) : fov(fov), _cached_matrix(1), current_mode(camera_mode)
{
    _update_cache();
}

glm::mat4 cr::camera::mat4() const noexcept
{
    return _cached_matrix;
}

cr::ray cr::camera::get_ray(float x, float y, float aspect)
{
    switch (current_mode)
    {
    case mode::perspective:
    {
        const auto u = (2.0f * x - 1.0f) * aspect;
        const auto v = 2.0f * y - 1.0f;
        const auto w = 1.0f / glm::tan(0.5f * glm::radians(fov));

        const auto direction = glm::vec3((_cached_matrix * glm::vec4(u, v, w, 0.0f)));

        return cr::ray(position, glm::normalize(direction));
    }
    case mode::orthographic:
    {
        const auto u = 2.0f * x - 1.0f;
        const auto v = 2.0f * y - 1.0f;

        const auto origin    = glm::vec3(_cached_matrix * glm::vec4(scale * u, scale * v, 0.0, 1.0));
        const auto direction = glm::vec3(_cached_matrix[2]);

        return cr::ray(origin, glm::normalize(direction));
    }
    }
}

void cr::camera::translate(const glm::vec3 &translation)
{
    position = glm::vec3(_cached_matrix * glm::vec4(translation, 1.0f));
    _update_cache();
}

void cr::camera::rotate(const glm::vec3 &rotation)
{
    this->rotation += rotation;
    this->rotation.y = glm::clamp(this->rotation.y, -90.f, 90.f);
    _update_cache();
}

void cr::camera::_update_cache()
{
    constexpr glm::vec3 UP = glm::vec3(0, 1, 0);
    constexpr glm::vec3 RIGHT = glm::vec3(1, 0, 0);
    constexpr glm::vec3 FORWARD = glm::vec3(0, 0, 1);

    auto mat = glm::mat4(1.f);

    mat = glm::translate(mat, position);
    mat = glm::rotate(mat, glm::radians(rotation.x), UP);
    mat = glm::rotate(mat, glm::radians(rotation.y), RIGHT);
    mat = glm::rotate(mat, glm::radians(rotation.z), FORWARD);

    _cached_matrix = mat;
}
