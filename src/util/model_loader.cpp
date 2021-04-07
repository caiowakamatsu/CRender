#include "model_loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tinobj.h>

cr::model::model_data cr::model::load_model(const std::string &file)
{
    auto model_data = cr::model::model_data();

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

        auto       material_info    = cr::material::information();
        static int current_material = 0;
        material_info.name =
          shape.name.empty() ? "Fallback Name - " + std::to_string(++current_material) : shape.name;
        material_info.type      = cr::material::type::smooth;
        material_info.emission  = 0;
        material_info.ior       = 0;
        material_info.roughness = 0;
        material_info.colour    = glm::vec3(0.2, 0.2, 0.2);

        auto material = cr::material(material_info);
        model_data.materials.push_back(std::move(material));

        model_data.material_indices.reserve(shape.mesh.indices.size() / 3);
        for (auto f = 0; f < shape.mesh.num_face_vertices.size(); f++)
            model_data.material_indices.push_back(model_data.materials.size() - 1);
    }

    return std::move(model_data);
}
