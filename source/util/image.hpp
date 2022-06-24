//
// Created by Caio W on 8/2/22.
//

#ifndef CREBON_IMAGE_HPP
#define CREBON_IMAGE_HPP

#include <atomic>
#include <cstdint>
#include <limits>
#include <vector>

#include <glm/glm.hpp>

#include <fmt/core.h>

#include <util/atomic_image.hpp>

namespace cr {

class image {
private:
  static constexpr auto ValType_Max = std::numeric_limits<float>::max();

public:
  image() = default;

  image(const std::vector<float> &data, uint64_t width, uint64_t height)
      : _width(width), _height(height) {
    if (data.size() != _width * _height * 4) {
      fmt::print("failed to create image with [{} {}], data was {}", _width,
                 _height, data.size());
      exit(-1);
    }

    _image_data = data;
  }

  image(uint64_t width, uint64_t height) : _width(width), _height(height) {
    _image_data = std::vector<float>(_width * _height * 4, ValType_Max);
  }

  void clear() {
    std::fill(_image_data.begin(), _image_data.end(), ValType_Max);
  }

  [[nodiscard]] bool valid() const noexcept {
    return _width != std::numeric_limits<std::uint64_t>::max() &&
           _height != std::numeric_limits<std::uint64_t>::max();
  }

  [[nodiscard]] float *data() noexcept { return _image_data.data(); }

  [[nodiscard]] const float *data() const noexcept {
    return _image_data.data();
  }

  [[nodiscard]] uint64_t width() const noexcept { return _width; }

  [[nodiscard]] uint64_t height() const noexcept { return _height; }

  [[nodiscard]] static cr::image
  from_float3_buffer(const std::vector<float> &buffer, size_t width,
                     size_t height) {
    auto image = cr::image(width, height);

    for (auto i = 0; i < width * height; i++) {
      image._image_data[i * 4 + 0] = buffer[i * 3 + 0];
      image._image_data[i * 4 + 1] = buffer[i * 3 + 1];
      image._image_data[i * 4 + 2] = buffer[i * 3 + 2];
      image._image_data[i * 4 + 3] = 1;
    }

    return image;
  }

  [[nodiscard]] std::vector<float> as_float3_buffer() const noexcept {
    auto output = std::vector<float>(_width * _height * 3);

    for (auto i = 0; i < _width * _height; i++) {
      output[i * 3 + 0] = _image_data[i * 4 + 0];
      output[i * 3 + 1] = _image_data[i * 4 + 1];
      output[i * 3 + 2] = _image_data[i * 4 + 2];
    }

    return output;
  }

  [[nodiscard]] std::vector<float> as_float4_buffer() const noexcept {
    auto output = std::vector<float>(_width * _height * 4);

    std::memcpy(output.data(), _image_data.data(),
                sizeof(float) * _width * _height * 4);

    return output;
  }

  [[nodiscard]] glm::vec4 get_uv(float u, float v) const noexcept {
    return get(static_cast<uint64_t>(u * _width) % _width,
               static_cast<uint64_t>(v * _height) % _height);
  }

  [[nodiscard]] glm::vec4 get(uint64_t x, uint64_t y) const noexcept {
    const auto base_index = (x + y * _width) * 4;

    return {_image_data[base_index + 0], _image_data[base_index + 1],
            _image_data[base_index + 2], _image_data[base_index + 3]};
  }

  void set(uint64_t x, uint64_t y, const glm::vec3 &colour) noexcept {
    set(x, y, glm::vec4(colour, 1.0f));
  }

  [[nodiscard]] glm::vec4 operator[](size_t index) const noexcept {
    return glm::vec4(_image_data[index * 4 + 0], _image_data[index * 4 + 1],
                     _image_data[index * 4 + 2], _image_data[index * 4 + 3]);
  }

  void set(uint64_t x, uint64_t y, const glm::vec4 &colour) noexcept {
    const auto base_index = (x + y * _width) * 4;

    _image_data[base_index + 0] = colour.r;
    _image_data[base_index + 1] = colour.g;
    _image_data[base_index + 2] = colour.b;
    _image_data[base_index + 3] = colour.a;
  }

  [[nodiscard]] image_view view() noexcept {
    return {
      .width = _width,
      .height = _height,
      .data = as_float3_buffer()
    };
  }

private:
  std::vector<float> _image_data;
  uint64_t _width = std::numeric_limits<uint64_t>::max();
  uint64_t _height = std::numeric_limits<uint64_t>::max();
};
} // namespace cr

#endif // CREBON_IMAGE_HPP
