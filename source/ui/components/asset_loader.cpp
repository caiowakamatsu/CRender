//
// Created by howto on 24/5/2022.
//

#include "asset_loader.hpp"

#include <imgui.h>

namespace cr::component {

asset_loader::Options
asset_loader::Component::display(asset_loader::DisplayContents contents) const {
  static std::string current_directory;
  if (ImGui::BeginCombo("Select Model", current_directory.c_str())) {
    for (const auto &entry : std::filesystem::directory_iterator("./assets/models"))
    {
      if (!entry.is_directory())
        continue;

    }
  }

  return {};
}

}
