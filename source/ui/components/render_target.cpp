//
// Created by howto on 23/5/2022.
//

#include "render_target.h"
namespace cr::component {
render_target::Options render_target::Component::display(
    render_target::DisplayContents contents) const {

  static auto size = glm::ivec2(1920, 1080);

  ImGui::InputInt2("Size", &size[0]);

  return {
      .resolution = size,
  };
}
} // namespace cr::component
