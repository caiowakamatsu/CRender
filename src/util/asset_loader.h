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

namespace cr::asset_loader
{
    struct material_index
    {
        uint32_t begin;
        uint32_t end;
    };

    struct loaded_model
    {
        std::string name;

        std::vector<glm::vec3> vertices;
        std::vector<material>  materials;
        std::vector<glm::vec2> texture_coords;
        std::vector<glm::vec3> normals;
        std::vector<cr::image> textures;

        std::vector<uint32_t> vertex_indices;
        std::vector<uint32_t> material_indices;
        std::vector<uint32_t> texture_indices;
        std::vector<uint32_t> normal_indices;
    };

    [[nodiscard]] loaded_model load_model(const std::string &file, const std::string &folder);

    struct picture_data
    {
        glm::ivec2         res;
        std::vector<float> colour;

        [[nodiscard]] inline cr::image as_image() { return cr::image(colour, res.x, res.y); }
    };

    [[nodiscard]] picture_data load_picture(const std::string &file);
    [[nodiscard]] std::optional<std::string>
      valid_directory(const std::filesystem::directory_entry &directory);

    [[nodiscard]] std::optional<std::string>
      valid_font(const std::filesystem::directory_entry &directory);

    enum class image_type
    {
        PNG,
        JPG,
        EXR,
        HDR,
    };
    void export_framebuffer(const cr::image &buffer, const std::string &path, image_type type);
}    // namespace cr::asset_loader
