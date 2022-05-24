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

  const auto sub_menu = [this](std::string_view name, sub_settings setting,
                               auto &interface, auto interface_data,
                               auto callback) {
    if (ImGui::Button(name.data())) {
      if (_selected == setting)
        _selected = std::nullopt;
      else
        _selected = setting;
    }

    if (_selected == setting)
      callback(interface.display(interface_data));
  };

  sub_menu("Render Target", sub_settings::render_target,
           _sub_settings.render_target, render_target::DisplayContents{},
           []([[maybe_unused]] auto options) {
             // TODO: Implement
           });

  sub_menu("Image Export", sub_settings::image_export,
           _sub_settings.image_export, image_export::DisplayContents{},
           []([[maybe_unused]] auto options) {
             // TODO: Implement
           });
/*
  if (ImGui::Button("Render Target")) {
    if (_selected == sub_settings::render_target)
      _selected = std::nullopt;
    else
      _selected = sub_settings::render_target;
  }

  if (_selected == sub_settings::render_target) {
    [[maybe_unused]] auto resolution =
        cr::component::interface<render_target>().display({});
  }

  if (ImGui::Button("Image Export")) {
    if (_selected == sub_settings::image_export)
      _selected = std::nullopt;
    else
      _selected = sub_settings::image_export;
  }

  if (_selected == sub_settings::image_export) {
    [[maybe_unused]] auto image_export_settings =
        cr::component::interface<image_export>().display({});
  }*/

  return {};
}
