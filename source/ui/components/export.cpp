//
// Created by howto on 24/5/2022.
//

#include "export.hpp"

#include <imgui.h>

namespace cr::component {

image_export::Options
image_export::Component::display(image_export::DisplayContents contents) const {
  static auto options = Options();

  // Todo: Fix issue with writing the image name moving camera around
  ImGui::InputText("Scene Name", options.scene_name.data(), 512);

  ImGui::Checkbox("Gamma Correct", &options.gamma_correct);

  return options;
}
} // namespace cr::component
