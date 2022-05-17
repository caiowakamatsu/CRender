//
// Created by Caio W on 12/2/22.
//

#include "settings.hpp"

#include <imgui/imgui.h>

cr::component::settings::Options
cr::component::settings::Component::display(DisplayContents contents) const {

	ImGui::Begin("Settings");

	ImGui::Text("Settings");
	ImGui::Separator();

	ImGui::End();

	return {};
}
