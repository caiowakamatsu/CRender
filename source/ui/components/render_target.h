//
// Created by howto on 23/5/2022.
//

#ifndef CREBON_RENDER_TARGET_H
#define CREBON_RENDER_TARGET_H

#include <glm/glm.hpp>
#include <imgui.h>

namespace cr::component {
class render_target {
public:
  struct Options {
    glm::ivec2 resolution;
  };
  struct DisplayContents {};

  struct Component {
    [[nodiscard]] render_target::Options
    display(DisplayContents contents) const;
  };
};
} // namespace cr::component

#endif // CREBON_RENDER_TARGET_H
