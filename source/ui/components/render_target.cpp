//
// Created by howto on 23/5/2022.
//

#include "render_target.h"
namespace cr::component {
render_target::Options render_target::Component::display(
    render_target::DisplayContents contents) const {
  static auto options = render_target::Options();

  ImGui::InputInt2("Size", &options.resolution[0]);
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("The resolution of the render");
  }

  ImGui::InputInt("Ray Depth", &options.ray_depth);
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Maximum amount of times a ray can bounce");
  }

  ImGui::InputInt("Target SPP", &options.samples_per_pixel);
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("0 for no target");
  }

  return options;
}
} // namespace cr::component
