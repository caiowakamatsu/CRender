//
// Created by Caio W on 5/2/22.
//

#ifndef CREBON_RANDOM_HPP
#define CREBON_RANDOM_HPP

#include <cstdint>
#include <random>

namespace cr {
class random {
public:
  explicit random(std::uint32_t state) {
    _state = state * 747796405u + 2891336453u;
    const auto word = ((_state >> ((_state >> 28u) + 4u)) ^ _state) * 277803737u;
    _state = (word >> 22u) ^ word;
  }

  [[nodiscard]] float next_float() {
    _state ^= (_state << 13);
    _state ^= (_state >> 17);
    _state ^= (_state << 5);
    return _state / 4294967296.0f;
  }

  [[nodiscard]] std::uint32_t next_int(std::uint32_t max) {
    return std::uint32_t(next_float() * max);
  }

private:
  std::uint32_t _state;
};
} // namespace cr

#endif // CREBON_RANDOM_HPP
