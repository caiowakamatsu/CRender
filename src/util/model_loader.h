#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <optional>

#include <glm/glm.hpp>
#include <render/material/material.h>
#include <util/exception.h>
#include <regex>

namespace cr::model_loader
{
    struct model_data
    {
        std::vector<glm::vec3> vertices;
        std::vector<material> materials;
        std::vector<glm::vec2> texture_coords;

        std::vector<uint32_t> vertex_indices;
        std::vector<uint32_t> material_indices;
        std::vector<uint32_t> texture_indices;
    };
    [[nodiscard]] model_data load(const std::string &file, const std::string &folder);

    [[nodiscard]] model_data load_obj(const std::string &file, const std::string &folder);

    [[nodiscard]] std::optional<std::string> valid_directory(const std::filesystem::directory_entry& directory);
}
