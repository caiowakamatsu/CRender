//
// Created by howto on 24/5/2022.
//

#include "asset_loader.hpp"

#include <imgui.h>

namespace cr::component {

asset_loader::Options
asset_loader::Component::display(asset_loader::DisplayContents contents) const {
  bool throw_away = false;

  static auto options = Options();

  static std::string current_directory;
  if (ImGui::BeginCombo("Select Model", current_directory.c_str())) {
    for (const auto &entry :
         std::filesystem::directory_iterator("./assets/models")) {
      if (entry.is_directory())
        continue;

      if (entry.path().extension() != ".glb")
        continue;

      if (ImGui::Selectable(entry.path().string().c_str(), &throw_away)) {
        current_directory = entry.path().string();
        options.load = entry.path().string();
      }
    }
    ImGui::EndCombo();
  }

  return options;
}

} // namespace cr::component
