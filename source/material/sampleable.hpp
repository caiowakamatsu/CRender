#ifndef CREBON_SAMPLEABLE_HPP
#define CREBON_SAMPLEABLE_HPP

#include <optional>
#include <variant>

#include <glm/vec3.hpp>

#include <util/image.hpp>

namespace {
[[nodiscard]] inline float from_vec3(glm::vec3 from) {
  return (from.x + from.y + from.z) / 3.0f;
}
} // namespace

namespace cr {
template <typename T> class sampleable {
private:
  std::optional<cr::image> _map;
  T _base;

public:
  explicit sampleable(T base) : _map(std::nullopt), _base(std::move(base)) {}

  explicit sampleable(cr::image map) : _map(std::move(map)), _base(-1.0f) {}

  [[nodiscard]] T sample(float u, float v) const noexcept = delete;
};

template <>
inline glm::vec3 sampleable<glm::vec3>::sample(float u,
                                               float v) const noexcept {
  return _map.has_value() ? _map->get_uv(u, v) : _base;
}

template <>
inline float sampleable<float>::sample(float u, float v) const noexcept {
  return _map.has_value() ? ::from_vec3(_map->get_uv(u, v)) : _base;
}
} // namespace cr

#endif // CREBON_SAMPLEABLE_HPP