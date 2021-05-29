#include "model_loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tinobj.h>

namespace
{
    // Thanks https://stackoverflow.com/a/42844629
    [[nodiscard]] bool ends_with(std::string_view str, std::string_view suffix)
    {
        return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
    }
}

cr::model_loader::model_data cr::model_loader::load(const std::string &file)
{
    auto model_data = cr::model_loader::model_data();

    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.mtl_search_path = "./";
    readerConfig.triangulate     = true;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(file, readerConfig) && !reader.Error().empty())
        cr::exit("Couldn't parse OBJ from file");

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

    for (const auto &shape : shapes)
    {
        model_data.vertex_indices.reserve(shape.mesh.indices.size());

        for (const auto idx : shape.mesh.indices)
            model_data.vertex_indices.push_back(idx.vertex_index);

        model_data.material_indices.reserve(shape.mesh.material_ids.size());
        for (auto i = 0; i < shape.mesh.material_ids.size(); i++)
            model_data.material_indices[i] = shape.mesh.material_ids[i];

        auto       material_info    = cr::material::information();
        static int current_material = 0;
        material_info.name =
          shape.name.empty() ? "Fallback Name - " + std::to_string(++current_material) : shape.name;
        material_info.type      = cr::material::type::smooth;
        material_info.ior       = 0;
        material_info.roughness = 0;
        material_info.reflectiveness = 0;
        material_info.colour    = glm::vec3(0.2, 0.2, 0.2);

        shape.mesh.material_ids;

        auto material = cr::material(material_info);
        model_data.materials.push_back(std::move(material));

        model_data.material_indices.reserve(shape.mesh.indices.size() / 3);
        for (auto f = 0; f < shape.mesh.num_face_vertices.size(); f++)
            model_data.material_indices.push_back(model_data.materials.size() - 1);
    }

    return std::move(model_data);
}
bool cr::model_loader::valid_directory(const std::filesystem::directory_entry &directory)
{
    // Model types accepted in a directory are
    // .obj
    // .schematic (still needs to be parsed properly)
    // none other lol
    auto obj_found = false;
    auto schem_found = false;

    for (const auto &entry : std::filesystem::directory_iterator(directory))
    {
        if (entry.is_regular_file())
        {
            const auto &entry_name = entry.path().string();
            if (ends_with(entry_name, ".obj"))
                obj_found |= true;
            else if (ends_with(entry_name, ".schematic"))
                schem_found |= true;
        }
    }

    return obj_found || schem_found;
}
