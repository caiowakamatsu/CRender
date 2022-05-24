//
// Created by howto on 25/5/2022.
//

#ifndef CREBON_SKY_HPP
#define CREBON_SKY_HPP

#include <glm/glm.hpp>

namespace cr::sky {
[[nodiscard]] glm::vec3 at(const glm::vec2 &uv, const glm::vec2 &sun_pos = {0.5f, 0.2f});
}

#endif // CREBON_SKY_HPP
