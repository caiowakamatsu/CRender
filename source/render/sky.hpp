//
// Created by howto on 25/5/2022.
//

#ifndef CREBON_SKYBOX_HPP
#define CREBON_SKY_HPP

#include <glm/glm.hpp>

#include <ui/components/skybox.hpp>

namespace cr {
class sky {
private:
  component::skybox::Options _options;

public:
  explicit sky(component::skybox::Options options);

  [[nodiscard]] glm::vec3 at(const glm::vec2 &uv,
                             const glm::vec2 &sun_pos = {0.5f, 0.2f});

  void use_settings(component::skybox::Options options);
};

} // namespace cr

#endif // CREBON_SKYBOX_HPP
