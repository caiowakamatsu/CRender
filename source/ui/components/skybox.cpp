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
}
