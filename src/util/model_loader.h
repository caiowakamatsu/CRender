#pragma once

#include <vector>
#include <string>
#include <fstream>

#include <glm/glm.hpp>
#include <render/material/material.h>
#include <util/exception.h>

namespace cr::model
{
    struct model_data
    {
        std::vector<glm::vec3> vertices;
        std::vector<material> materials;

        std::vector<uint64_t> vertex_indices;
        std::vector<uint32_t> material_indices;
    };
    [[nodiscard]] model_data load_model(const std::string &file);
}
