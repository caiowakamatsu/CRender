//
// Created by howto on 24/5/2022.
//

#include "export.hpp"

#include <imgui.h>
#include <array>

namespace cr::component {

image_export::Options
image_export::Component::display(image_export::DisplayContents contents) const {
  static auto options = Options();

  static auto file_string = std::array<char, 65>();
  // Todo: Fix issue with writing the image name moving camera around
  ImGui::InputText("Scene Name", file_string.data(), 64);
  options.scene_name = file_string.data();

  ImGui::Checkbox("Gamma Correct", &options.gamma_correct);

  return options;
}
} // namespace cr::component
