//
// Created by Caio W on 5/2/22.
//

#ifndef CREBON_CONFIGURATION_HPP
#define CREBON_CONFIGURATION_HPP

#include <scene/ray.hpp>
#include <util/random.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include <concurrentqueue/concurrentqueue.h>

namespace cr {
class scene_configuration {
private:
  float _fov;
  uint64_t _width;
  uint64_t _height;
  uint64_t _bounces;
  glm::vec3 _origin;
  glm::vec3 _rotation;
  glm::mat4 _pv_matrix;

public:
  scene_configuration(const glm::vec3 &origin, const glm::vec3 rotation,
                      uint64_t width, uint64_t height, float fov,
                      uint64_t bounces)
      : _pv_matrix(1.0f), _width(width), _height(height), _rotation(rotation),
        _fov(fov), _bounces(bounces), _origin(origin) {
    auto view_matrix = glm::mat4(1);
    view_matrix = glm::translate(view_matrix, origin);

    constexpr auto RIGHT = glm::vec3(1, 0, 0);
    constexpr auto UP = glm::vec3(0, 1, 0);
    constexpr auto FORWARD = glm::vec3(0, 0, 1);

    view_matrix = glm::rotate(view_matrix, glm::radians(rotation.x), UP);
    view_matrix = glm::rotate(view_matrix, glm::radians(rotation.y), RIGHT);
    view_matrix = glm::rotate(view_matrix, glm::radians(rotation.z), FORWARD);

    _pv_matrix = view_matrix;
  }

  [[nodiscard]] glm::mat4 matrix() const noexcept { return _pv_matrix; }

  [[nodiscard]] glm::mat4 rotation_matrix() const noexcept {
    constexpr auto RIGHT = glm::vec3(1, 0, 0);
    constexpr auto UP = glm::vec3(0, 1, 0);
    constexpr auto FORWARD = glm::vec3(0, 0, 1);

    auto rotation_matrix = glm::mat4(1.0f);
    rotation_matrix =
        glm::rotate(rotation_matrix, glm::radians(_rotation.x), UP);
    rotation_matrix =
        glm::rotate(rotation_matrix, glm::radians(_rotation.y), RIGHT);
    rotation_matrix =
        glm::rotate(rotation_matrix, glm::radians(_rotation.z), FORWARD);

    return rotation_matrix;
  }

  [[nodiscard]] uint64_t width() const noexcept { return _width; }

  [[nodiscard]] uint64_t height() const noexcept { return _height; }

  [[nodiscard]] float fov() const noexcept { return _fov; }

  [[nodiscard]] uint64_t bounces() const noexcept { return _bounces; }

  [[nodiscard]] glm::vec3 origin() const noexcept { return _origin; }

  [[nodiscard]] glm::vec3 rotation() const noexcept { return _rotation; }

  [[nodiscard]] float aspect() const noexcept {
    return static_cast<float>(_width) / static_cast<float>(_height);
  }

  using MinMaxPair = std::pair<glm::ivec2, glm::ivec2>;
  [[nodiscard]] std::vector<MinMaxPair> get_tasks(size_t device_count) const {
    // If you have a scene that's too small and this breaks, that's on you :)
    const auto chunk_width = _width / device_count;
    const auto chunk_height = _height / device_count;

    auto tasks = std::vector<MinMaxPair>();

    // Create tasks
    for (size_t x = 0; x < device_count; x++) {
      for (size_t y = 0; y < device_count; y++) {
        const auto min_x = x * chunk_width;
        const auto min_y = y * chunk_height;
        const auto max_x = min_x + chunk_width;
        const auto max_y = min_y + chunk_height;

        tasks.push_back(
            MinMaxPair(glm::ivec2(min_x, min_y), glm::ivec2(max_x, max_y)));
      }
    }

    return tasks;
  }

  [[nodiscard]] cr::ray ray(uint64_t x, uint64_t y,
                            cr::random &rand) const noexcept {
    const auto u =
        (((static_cast<float>(x) + rand.next_float()) / _width) * 2.0f - 1.0f) *
        aspect();
    const auto v =
        ((static_cast<float>(y) + rand.next_float()) / _height) * 2.0f - 1.0f;
    const auto w = 1.0f / glm::tan(0.5 * glm::radians(_fov));

    const auto direction = glm::vec3(_pv_matrix * glm::vec4(u, v, w, 0.0f));

    return {_origin, glm::normalize(direction)};
  }
};
} // namespace cr

#endif // CREBON_CONFIGURATION_HPP
