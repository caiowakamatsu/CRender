//
// Created by howto on 25/5/2022.
//

#include "skybox.hpp"

#include <imgui.h>

namespace cr::component {

skybox::Options
skybox::Component::display(skybox::DisplayContents contents) const {
  static auto options = Options{};

  ImGui::SliderFloat("Sun Radius", &options.sunRadius, 1.0f, 100.0f);
  ImGui::SliderFloat("Sun Radiance", &options.sunRadiance, 0.0f, 100.0f);

  static auto use_angles = true;
  ImGui::Checkbox("Use Angles", &use_angles);

  static auto sun_dir = glm::vec3(0.8f, -1.0f, 0.0f);

  if (use_angles) {
    ImGui::SliderFloat("Elevation", &sun_dir.x, -90, 90);
    ImGui::SliderFloat("Azimuth", &sun_dir.y, -180, 180);
  } else {
    ImGui::InputFloat("Direction X", &sun_dir.x);
    ImGui::InputFloat("Direction Y", &sun_dir.y);
    ImGui::InputFloat("Direction Z", &sun_dir.z);
  }

  const auto elevation = -sun_dir.x / 360.0f * std::numbers::pi_v<float> * 2.0f;
  const auto azimuth = sun_dir.y / 360.0f * std::numbers::pi_v<float> * 2.0f;

  auto dir = glm::vec3();
  if (use_angles) {
    dir = glm::normalize(glm::vec3(std::cos(elevation) * std::cos(azimuth),
                                   std::sin(elevation),
                                   std::cos(elevation) * std::sin(azimuth)));
  } else {
    dir = glm::normalize(sun_dir);
  }
  // translate sun_dir from a direction to cartesian coordinates
  const auto uv = glm::vec2(
      0.5f + std::atan2(dir.z, dir.x) / (2.0f * std::numbers::pi_v<float>),
      0.5f - std::asin(dir.y) / std::numbers::pi_v<float>);
  options.sun_pos = uv;

  ImGui::NewLine();
  ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Advanced Options");
  ImGui::InputFloat("Earth Radius", &options.earthRadius);
  ImGui::InputFloat("Earth Atmosphere Top Radius", &options.earthAtmTopRadius);
  ImGui::NewLine();
  ImGui::InputFloat("mieG", &options.mieG);
  ImGui::InputFloat("mieHeight", &options.mieHeight);
  ImGui::InputFloat("rayleighHeight", &options.rayleighHeight);

  return options;
}
} // namespace cr::component
