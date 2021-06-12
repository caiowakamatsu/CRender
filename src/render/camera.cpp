#include "camera.h"

cr::camera::camera(glm::vec3 position, float fov) :
position(position), fov(fov)
{

}

glm::mat4 cr::camera::mat4() const noexcept
{
    return _cached_matrix;
}

cr::ray cr::camera::get_ray(float x, float y)
{
    const auto u = 2.0f * x - 1.0f;
    const auto v = 2.0f * y - 1.0f;
    const auto w = 1.0f / glm::tan(0.5f * fov);

    const auto direction = glm::vec3((_cached_matrix * glm::vec4(u, v, w, 0.0f)));

    return cr::ray(position, direction);
}
void cr::camera::translate(const glm::vec3 &translation)
{
    position += glm::vec3(_cached_matrix * glm::vec4(translation, 0.0f));
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
    auto mat = glm::mat4(1.f);

    mat = glm::translate(mat, position);
    const auto rot = glm::eulerAngleXYZ(rotation.y, rotation.z, rotation.x);

    _cached_matrix = rot * mat;
}
