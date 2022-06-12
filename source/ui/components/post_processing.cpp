//
// Created by Caio W on 11/6/22.
//

#include "post_processing.hpp"

#include <imgui.h>

namespace cr::component {

post_processing::Options
post_processing::Component::display(post_processing::DisplayContents contents) const {
  static auto options = Options();

  ImGui::Checkbox("Gamma Correction", &options.gamma_correct);
  ImGui::InputFloat("Exposure", &options.exposure);

  return options;
}
} // namespace cr::component
