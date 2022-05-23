//
// Created by Caio W on 12/2/22.
//

#include "settings.hpp"

#include <ui/components/render_target.h>
#include <ui/components/component.hpp>

cr::component::settings::Options
cr::component::settings::Component::display(DisplayContents contents) const {

	ImGui::Begin("Settings");

	ImGui::Text("Settings");
	ImGui::Separator();

        static auto radio = false;
        if (false) {
          [[maybe_unused]] auto resolution = cr::component::interface<render_target>().display({});
        }

	ImGui::End();

	return {};
}
