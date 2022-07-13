//
// Created by howto on 24/5/2022.
// Modified by Yohwllo
//

#include "export.hpp"

#include <imgui.h>
#include <array>

namespace cr::component
{

  image_export::Options
  image_export::Component::display(image_export::DisplayContents contents) const
  {
    static auto options = Options();

    static auto file_string = std::array<char, 65>();
    // Todo: Fix issue with writing the image name moving camera around
    ImGui::InputText("Scene Name", file_string.data(), 64);


    // Don't really need to use BeginCombo thing, this works just fine 
    const char *file_types[] = {"JPG", "OpenEXR"};
    // I have no clue why I need this int
    static int selection = 0;
    ImGui::Combo("Image Export Type", &selection, file_types, IM_ARRAYSIZE(file_types));

    options.scene_name = file_string.data();
    options.image_type = selection;

    return options;
  }
} // namespace cr::component
