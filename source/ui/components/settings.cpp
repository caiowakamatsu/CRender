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
      } else {
        _selected = setting;
        return true;
      }
    } else {
      return _selected == setting;
    }
  };

  auto opts = Options();

  if (sub_menu("Render Target", sub_settings::render_target)) {
    const auto render_target_settings = _sub_settings.render_target.display({});
    if (ImGui::Button("Apply")) {
      opts.render_target = render_target_settings;
    }
  }

  if (sub_menu("Image Export", sub_settings::image_export)) {
    const auto image_export_settings = _sub_settings.image_export.display({});
    if (ImGui::Button("Apply")) {
      opts.image_export = image_export_settings;
    }
  }

  if (sub_menu("Asset Loader", sub_settings::asset_loader)) {
    const auto asset_loader_settings = _sub_settings.asset_loader.display({});
    if (ImGui::Button("Load")) {
      opts.asset_loader = asset_loader_settings;
    }
  }

  if (sub_menu("Skybox", sub_settings::skybox)) {
    const auto skybox_settings = _sub_settings.skybox.display({});
    if (ImGui::Button("Apply")) {
      opts.skybox = skybox_settings;
    }
  }

  ImGui::End();
  return opts;
}
