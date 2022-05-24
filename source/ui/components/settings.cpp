//
// Created by Caio W on 12/2/22.
//

#include "settings.hpp"

#include <string_view>

cr::component::settings::Options
cr::component::settings::Component::display(DisplayContents contents) {
  ImGui::Begin("Settings");

  ImGui::Text("Settings");
  ImGui::Separator();

  const auto sub_menu = [this](std::string_view name, sub_settings setting) {
    if (ImGui::Button(name.data())) {
      if (_selected == setting) {
        _selected = std::nullopt;
        return false;
      }
      else {
        _selected = setting;
        return true;
      }
    } else {
      return _selected == setting;
    }
  };

  if (sub_menu("Render Target", sub_settings::render_target)) {
    [[maybe_unused]] const auto render_target_settings = _sub_settings.render_target.display({});
  }

  if (sub_menu("Image Export", sub_settings::image_export)) {
    [[maybe_unused]] const auto image_export_settings = _sub_settings.image_export.display({});
  }

  if (sub_menu("Asset Loader", sub_settings::asset_loader)) {
    [[maybe_unused]] const auto asset_loader_settings = _sub_settings.asset_loader.display({});
  }

  return {};
}
