#include "model_loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tinobj.h>

#include <obj_loader/OBJ_Loader.h>

#include <stb/stb_image.h>

namespace
{
    // Thanks https://stackoverflow.com/a/42844629
    [[nodiscard]] bool ends_with(const std::string_view &str, const std::string_view &suffix)
    {
        return str.size() >= suffix.size() &&
          0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
    }

}    // namespace

cr::model_loader::model_data
  cr::model_loader::load(const std::string &file, const std::string &folder)
{
    auto model_data = cr::model_loader::model_data();

    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.triangulate = true;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(file, readerConfig) && !reader.Error().empty())
        cr::exit("Couldn't parse OBJ from file");
    fmt::print("Obj stuff: [{}]", reader.Warning());

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

    for (const auto &material : materials)
    {
        auto material_data = cr::material::information();
        material_data.name = material.name;
        material_data.colour =
          glm::vec3(material.diffuse[0], material.diffuse[1], material.diffuse[2]);
        material_data.type     = cr::material::type::smooth;
        material_data.emission = 0.0f;

        // Texture stuff!
        if (!material.diffuse_texname.empty())
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
            material_data.tex = std::move(texture_image);
        }

        model_data.materials.emplace_back(material_data);
    }

    for (const auto &shape : shapes)
    {
        for (const auto idx : shape.mesh.indices)
        {
            if (idx.vertex_index == -1)
                cr::exit("Vertex index was -1");
            if (idx.texcoord_index == -1)
                cr::exit("Tex coord index was -1");
            if (idx.normal_index == -1)
                cr::exit("Normal index was -1");

            model_data.vertex_indices.push_back(idx.vertex_index);

            model_data.texture_indices.push_back(idx.texcoord_index);

            model_data.normal_indices.push_back(idx.normal_index);
        }

        for (auto material_id : shape.mesh.material_ids)
        {
            model_data.material_indices.push_back(material_id);
        }
    }

    //    model_data.texture_coords =
    //      ::fix(model_data.texture_coords, model_data.vertex_indices, model_data.texture_indices);

    return model_data;
}

std::optional<std::string>
  cr::model_loader::valid_directory(const std::filesystem::directory_entry &directory)
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
