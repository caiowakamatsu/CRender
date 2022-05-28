//
// Created by howto on 29/5/2022.
//

#include "stats.hpp"

#include <imgui.h>

namespace cr::component {

stats::Options
stats::Component::display(stats::DisplayContents contents) const {
  ImGui::Begin("Stats");

  ImGui::Text("Sample per second: %d", contents.samples_per_second);
  ImGui::Text("Sample Count: %d", contents.total_samples);
  ImGui::NewLine();

  ImGui::Text("Rays per second: %d", contents.rays_per_second);
  ImGui::NewLine();

  ImGui::Text("Total Instances: %d", contents.total_instances);
  ImGui::NewLine();

  ImGui::Text("Total Render Time: %f", contents.total_render_time);
  ImGui::NewLine();

  ImGui::End();

  return stats::Options();
}
}
