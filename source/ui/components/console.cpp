//
// Created by Caio W on 12/2/22.
//

#include "console.hpp"

#include <imgui.h>

cr::component::console::Options
cr::component::console::Component::display(DisplayContents contents) const {
  ImGui::Begin("Console");

  ImGui::Text("Console Output");
  ImGui::Separator();

  ImGui::BeginChild("Console-Text-Region");

  static auto lines_to_display = std::vector<std::string>();
  lines_to_display.reserve(lines_to_display.size() + contents.lines->size());
  for (const auto &string : *contents.lines)
    lines_to_display.push_back(string);

  for (const auto &line : lines_to_display)
    ImGui::Text("%s", line.c_str());

  if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    ImGui::SetScrollHereY(1.0f);

  ImGui::EndChild();

  ImGui::End();

  return {};
}
