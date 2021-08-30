#include "asset_loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tinobj.h>

#include <obj_loader/OBJ_Loader.h>

#include <stb/stb_image.h>
#include <stb/stbi_image_write.h>

#define TINYEXR_IMPLEMENTATION
#include <tinyexr/tinyexr.h>

#include <util/logger.h>

namespace
{
    // Thanks https://stackoverflow.com/a/42844629
    [[nodiscard]] bool ends_with(const std::string_view &str, const std::string_view &suffix)
    {
        return str.size() >= suffix.size() &&
          0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
    }

    [[nodiscard]] cr::asset_loader::picture_data load_exr(const std::filesystem::path &path)
    {
        float *     raw_data;
        auto        dimension = glm::ivec2();
        const char *err       = nullptr;

        auto ret = LoadEXR(&raw_data, &dimension.x, &dimension.y, path.string().c_str(), &err);

        if (ret != TINYEXR_SUCCESS)
            cr::exit(
              fmt::format("There was an error loading EXR image {}", path.filename().string()));

        auto data = std::vector<float>(dimension.x * dimension.y * 4);

        std::memcpy(data.data(), raw_data, dimension.x * dimension.y * 4 * sizeof(float));

        // This is a C api im sorry
        free(raw_data);

        return { dimension, std::move(data) };
    }

    [[nodiscard]] cr::asset_loader::picture_data load_hdr(const std::filesystem::path &path)
    {
        auto image_dimensions = glm::ivec3();
        auto data             = stbi_loadf(
          path.string().c_str(),
          &image_dimensions.x,
          &image_dimensions.y,
          &image_dimensions.z,
          4);

        auto output = std::vector<float>(image_dimensions.x * image_dimensions.y * 4);
        std::memcpy(
          output.data(),
          data,
          image_dimensions.x * image_dimensions.y * 4 * sizeof(float));

        stbi_image_free(data);
        auto dim = glm::vec2(image_dimensions.x, image_dimensions.y);
        return { dim, std::move(output) };
    }

    [[nodiscard]] cr::asset_loader::picture_data load_jpg_png(const std::filesystem::path &path)
    {
        auto image_dimensions = glm::ivec3();
        auto data             = stbi_load(
          path.string().c_str(),
          &image_dimensions.x,
          &image_dimensions.y,
          &image_dimensions.z,
          4);

        auto output = std::vector<float>(image_dimensions.x * image_dimensions.y * 4);

        for (auto i = 0; i < image_dimensions.x * image_dimensions.y * 4; i++)
            output[i] = data[i] / 255.f;
        stbi_image_free(data);

        auto dim = glm::vec2(image_dimensions.x, image_dimensions.y);

        return { dim, std::move(output) };
    }

    void export_png(const cr::image &buffer, const std::string &path)
    {
        auto data = std::vector<uint8_t>(buffer.width() * buffer.height() * 4);
        for (auto i = 0; i < data.size(); i++) data[i] = glm::min(buffer.data()[i] * 255.f, 255.f);

        stbi_write_png(
          path.c_str(),
          buffer.width(),
          buffer.height(),
          4,
          data.data(),
          buffer.width() * 4);
    }

    void export_jpg(const cr::image &buffer, const std::string &path)
    {
        auto data = std::vector<uint8_t>(buffer.width() * buffer.height() * 4);
        for (auto i = 0; i < data.size(); i++) data[i] = glm::min(buffer.data()[i] * 255.f, 255.f);

        stbi_write_jpg(path.c_str(), buffer.width(), buffer.height(), 4, data.data(), 100);
    }

    void export_exr(const cr::image &buffer, const std::string &path)
    {
        EXRHeader header;
        InitEXRHeader(&header);

        EXRImage image;
        InitEXRImage(&image);

        image.num_channels = 3;

        std::vector<float> images[3];
        const auto         element_count = buffer.width() * buffer.height();
        images[0].resize(element_count);
        images[1].resize(element_count);
        images[2].resize(element_count);

        // Split RGBRGBRGB... into R, G and B layer
        for (int i = 0; i < element_count; i++)
        {
            images[0][i] = buffer.data()[4 * i + 0];
            images[1][i] = buffer.data()[4 * i + 1];
            images[2][i] = buffer.data()[4 * i + 2];
        }

        float *image_ptr[3];
        image_ptr[0] = &(images[2].at(0));    // B
        image_ptr[1] = &(images[1].at(0));    // G
        image_ptr[2] = &(images[0].at(0));    // R

        image.images = (unsigned char **) image_ptr;
        image.width  = buffer.width();
        image.height = buffer.height();

        header.num_channels = 3;
        header.channels = (EXRChannelInfo *) malloc(sizeof(EXRChannelInfo) * header.num_channels);
        // Must be (A)BGR order, since most of EXR viewers expect this channel order.
        strncpy(header.channels[0].name, "B", 255);
        header.channels[0].name[strlen("B")] = '\0';
        strncpy(header.channels[1].name, "G", 255);
        header.channels[1].name[strlen("G")] = '\0';
        strncpy(header.channels[2].name, "R", 255);
        header.channels[2].name[strlen("R")] = '\0';

        header.pixel_types           = (int *) malloc(sizeof(int) * header.num_channels);
        header.requested_pixel_types = (int *) malloc(sizeof(int) * header.num_channels);
        for (int i = 0; i < header.num_channels; i++)
        {
            header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;    // pixel type of input image
            header.requested_pixel_types[i] =
              TINYEXR_PIXELTYPE_HALF;    // pixel type of output image to be stored in .EXR
        }

        const char *err = NULL;    // or nullptr in C++11 or later.
        int         ret = SaveEXRImageToFile(&image, &header, path.c_str(), &err);
        if (ret != TINYEXR_SUCCESS) cr::logger::error("EXR export failed with error [{}]", err);

        free(header.channels);
        free(header.pixel_types);
        free(header.requested_pixel_types);
    }

    void export_hdr(const cr::image &buffer, const std::string &path)
    {
        auto data = std::vector<float>(buffer.width() * buffer.height() * 4);
        for (auto i = 0; i < data.size(); i++) data[i] = glm::pow(buffer.data()[i], 2.2f);

        stbi_write_hdr(path.c_str(), buffer.width(), buffer.height(), 4, data.data());
    }

}    // namespace

cr::asset_loader::loaded_model
  cr::asset_loader::load_model(const std::string &file, const std::string &folder)
{
    auto model_data = cr::asset_loader::loaded_model();
    model_data.name = std::filesystem::path(file).filename().stem().string();

    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.triangulate = true;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(file, readerConfig) && !reader.Error().empty())
        cr::exit("Couldn't parse OBJ from file");

    auto &attrib    = reader.GetAttrib();
    auto &shapes    = reader.GetShapes();
    auto &materials = reader.GetMaterials();

    // Copy the vertices into our buffer
    model_data.vertices.resize(attrib.vertices.size() / 3);
    for (auto i = 0; i < model_data.vertices.size(); i++)
    {
        model_data.vertices[i] = glm::vec3(
          attrib.vertices[i * 3 + 0],
          attrib.vertices[i * 3 + 1],
          attrib.vertices[i * 3 + 2]);
    }

    model_data.texture_coords.resize(attrib.texcoords.size() / 2);
    for (auto i = 0; i < model_data.texture_coords.size(); i++)
    {
        model_data.texture_coords[i] =
          glm::vec2(attrib.texcoords[i * 2 + 0], attrib.texcoords[i * 2 + 1]);
    }

    model_data.normals.resize(attrib.normals.size() / 3);
    for (auto i = 0; i < model_data.normals.size(); i++)
    {
        model_data.normals[i] = glm::vec3(
          attrib.normals[i * 3 + 0],
          attrib.normals[i * 3 + 1],
          attrib.normals[i * 3 + 2]);
    }

    auto already_loaded = std::unordered_map<std::string, uint32_t>();

    for (const auto &material : materials)
    {
        auto material_data = cr::material::information();
        material_data.name = material.name;
        material_data.colour =
          glm::vec4(material.diffuse[0], material.diffuse[1], material.diffuse[2], 1.0f);
        material_data.shade_type     = cr::material::type::smooth;
        material_data.emission = 0.0f;

        // Texture stuff!
        auto colour = glm::vec3();
        if (material.name == "Back_Wall" || material.name == "Material")
            colour = glm::vec3(1.0);
        else if (material.name == "Bottom_Plane") {
            material_data.roughness = .25f;
            material_data.shade_type = material::type::metal;
            colour = glm::vec3(0.227);
        } else if (material.name == "Middle_Plane") {
            material_data.roughness = .4;
            material_data.shade_type = material::type::metal;
            colour = glm::vec3(0.227);
        } else if (material.name == "Top_Plane") {
            material_data.roughness = .8f;
            material_data.shade_type = material::type::metal;
            colour = glm::vec3(0.227);
        }
        else if (material.name == "Big_Cube") {
            material_data.emission = 1.0f;
            colour = glm::vec3(0.25, 0, 1.0);
        }
        else if (material.name == "Medium_Cube") {
            material_data.emission = 1.0f;
            colour = glm::vec3(0.0, 0.1, 1.0);
        }
        else if (material.name == "Small_Cube") {
            material_data.emission = 1.0f;
            colour = glm::vec3(0.0, 0.95, 1.0);
        }

        material_data.colour = glm::vec4(colour, 1.0f);

        if (false)
            if (!material.diffuse_texname.empty())
            {
                if (const auto &it = already_loaded.find(material.diffuse_texname);
                    it == already_loaded.end())
                {
                    const auto texture_name = folder + '\\' + material.diffuse_texname;

                auto image_dimensions = glm::ivec3();
                stbi_set_flip_vertically_on_load(true);
                auto data = stbi_load(
                  texture_name.c_str(),
                  &image_dimensions.x,
                  &image_dimensions.y,
                  &image_dimensions.z,
                  4);
                stbi_set_flip_vertically_on_load(false);

                if (data != nullptr)
                {
                    auto texture_image = cr::image(image_dimensions.x, image_dimensions.y);

                    for (auto x = 0; x < image_dimensions.x; x++)
                        for (auto y = 0; y < image_dimensions.y; y++)
                        {
                            const auto base_index = (x + y * image_dimensions.x) * 4;

                            const auto r = data[base_index + 0] / 255.f;
                            const auto g = data[base_index + 1] / 255.f;
                            const auto b = data[base_index + 2] / 255.f;
                            const auto a = data[base_index + 3] / 255.f;

                            texture_image.set(x, y, glm::vec4(r, g, b, a));
                        }

                    stbi_image_free(data);
                    model_data.textures.push_back(std::move(texture_image));
                    material_data.tex = model_data.textures.size() - 1;
                    already_loaded.insert({ material.diffuse_texname, material_data.tex.value() });
                } else
                    cr::logger::warn("Failed to find texture [{}], defaulting to blank material", material.diffuse_texname);
            } else
                material_data.tex = it->second;
        }

        model_data.materials.emplace_back(material_data);
    }

    for (const auto &shape : shapes)
    {
        for (const auto idx : shape.mesh.indices)
        {
            model_data.vertex_indices.push_back(idx.vertex_index);

            model_data.texture_indices.push_back(idx.texcoord_index);

            model_data.normal_indices.push_back(idx.normal_index);
        }

        auto current_begin = 0;
        for (auto i = 0; i < shape.mesh.material_ids.size(); i++)
        {
            const auto material_id = shape.mesh.material_ids[i];

            if (i > 0 && material_id != shape.mesh.material_ids[i - 1])
            {
                auto idx = material_index();
                idx.begin = current_begin;
                idx.end = i;
                model_data.unique_material_indices.insert({material_id, idx});

                current_begin = i;
            }

            model_data.material_indices.push_back(material_id);
        }
        auto final_idx = material_index();
        final_idx.begin = current_begin;
        final_idx.end = shape.mesh.material_ids.size() - 1;
        model_data.unique_material_indices.insert({shape.mesh.material_ids[final_idx.end], final_idx});
    }

    return model_data;
}

cr::asset_loader::picture_data cr::asset_loader::load_picture(const std::string &file)
{
    auto path = std::filesystem::path(file);

    auto extension = path.extension().string();

    if (extension == ".exr")
        return load_exr(path);
    else if (extension == ".jpg" || extension == ".png")
        return load_jpg_png(path);
    else if (extension == ".hdr" || extension == ".hdri")
        return load_hdr(path);

    return {};
}

std::optional<std::string>
  cr::asset_loader::valid_directory(const std::filesystem::directory_entry &directory)
{
    // Model types accepted in a directory are
    // .obj
    // none other lol

    for (const auto &entry : std::filesystem::directory_iterator(directory))
    {
        if (entry.is_regular_file())
        {
            const auto &entry_name = entry.path().string();

            if (ends_with(entry_name, ".obj")) return entry_name;
        }
    }
    return {};
}

std::optional<std::string>
  cr::asset_loader::valid_font(const std::filesystem::directory_entry &directory)
{
    if (ends_with(directory.path().filename().extension().string(), ".ttf"))
        return directory.path().string();
    return {};
}

void cr::asset_loader::export_framebuffer(
  const cr::image &            buffer,
  const std::string &          path,
  cr::asset_loader::image_type type)
{
    auto extension = std::string(
      type == image_type::JPG     ? ".jpg"
        : type == image_type::PNG ? ".png"
        : type == image_type::EXR ? ".exr"
        : type == image_type::HDR ? ".hdr"
                                  : "");

    if (!extension.empty())
    {
        // Checking if the file already exists
        auto directory      = std::string("./out/") + path.data() + extension;
        auto attempt_number = 1;
        while (std::filesystem::exists(directory))
            directory = std::string("./out/") + path.data() + ' ' + "(" +
              std::to_string(attempt_number++) + ")" + extension;

        switch (type)
        {
        case image_type::PNG: ::export_png(buffer, directory); break;
        case image_type::JPG: ::export_jpg(buffer, directory); break;
        case image_type::EXR: ::export_exr(buffer, directory); break;
        case image_type::HDR: ::export_hdr(buffer, directory); break;
        }
    }
}
