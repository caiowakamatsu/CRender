//
// Created by howto on 24/5/2022.
//

#include "export.hpp"

#include <imgui.h>

namespace cr::component {

image_export::Options
image_export::Component::display(image_export::DisplayContents contents) const {

  int temp = 0;
  ImGui::SliderInt("Width", &temp, 1, 1024);

  return {};
}
} // namespace cr::component
