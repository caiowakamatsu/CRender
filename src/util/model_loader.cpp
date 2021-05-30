#include "model_loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tinobj.h>

namespace
{
    // Thanks https://stackoverflow.com/a/42844629
    [[nodiscard]] bool ends_with(const std::string_view &str, const std::string_view &suffix)
    {
        return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
    }
}

cr::model_loader::model_data cr::model_loader::load(const std::string &file)
{
    auto model_data = cr::model_loader::model_data();

    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.triangulate     = true;

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
        model_data.vertices[i] = glm::vec3(
          attrib.vertices[i * 3 + 0],
          attrib.vertices[i * 3 + 1],
          attrib.vertices[i * 3 + 2]);

    for (const auto &material : materials)
    {
        auto material_data = cr::material::information();
        material_data.name = material.name;
        material_data.colour = glm::vec3(
            material.diffuse[0],
            material.diffuse[1],
            material.diffuse[2]);
        material_data.type = cr::material::type::smooth;
        material_data.emission = 0.0f;

        model_data.materials.emplace_back(material_data);
    }

    for (const auto &shape : shapes)
    {
        for (const auto idx : shape.mesh.indices)
            model_data.vertex_indices.push_back(idx.vertex_index);

        for (int material_id : shape.mesh.material_ids)
            model_data.material_indices.push_back(material_id);
    }

    return std::move(model_data);
}
std::optional<std::string> cr::model_loader::valid_directory(const std::filesystem::directory_entry &directory)
{
    // Model types accepted in a directory are
    // .obj
    // none other lol

    for (const auto &entry : std::filesystem::directory_iterator(directory))
    {
        if (entry.is_regular_file())
        {
            const auto &entry_name = entry.path().string();

            if (ends_with(entry_name, ".obj"))
                return entry_name;
        }
    }
    return {};
}
