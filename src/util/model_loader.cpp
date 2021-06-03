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

    [[nodiscard]] std::vector<glm::vec2> fix(
      const std::vector<glm::vec2> &vtc,    // texture coordinates buffer
      const std::vector<uint32_t> &      vc,     // vertex coordinates index buffer
      const std::vector<uint32_t> &      vt      // vertex textures coordinates index buffer
    )
    {
        // read vtc, vc, and vt from file...

        // the code below reorders the texture coordinates in vtc_new so that the coordinates index
        // buffer may be used and you don't have to keep 2 index buffers
        std::vector<uint32_t>       vtci(vtc.size());       // map from vt to vc indexing
        std::vector<glm::vec2> vtc_new(vtc.size());    // reordered texture coordinates

        // construct mapping
        for (size_t i = 0; i < vt.size(); ++i)
            if (vtci[vt[i]] != -1 && vtci[vt[i]] != vc[i])
            {
                printf("No reordering possible.\n");
                printf("rel1: vt: %d, vc: %d\n", vt[i] - 1, vtci[vt[i] - 1]);
                printf("rel2: vt: %d, vc: %d\n", vt[i] - 1, vc[i] - 1);
                return vtc;
            }
            else
                vtci[vt[i]] = vc[i];
        // reorder texture coordinates
        for (size_t i = 0; i < vtc.size(); ++i) vtc_new[i] = vtc[vtci[i]];

        return vtc_new;
    }

    [[nodiscard]] std::vector<glm::vec2> reorder(
      std::vector<glm::vec2> tex_coords,
      std::vector<uint32_t>  vertex_coords_index,
      std::vector<uint32_t>  tex_coords_index)
    {
        auto vt_ci      = std::vector<uint32_t>(tex_coords.size());
        auto re_ordered = std::vector<glm::vec2>(tex_coords.size());

        for (auto &i : vt_ci) i = -1;

        for (auto i = 0; i < tex_coords.size(); ++i)
            if (
              vt_ci[tex_coords_index[i] - 1] != -1 &&
              vt_ci[tex_coords_index[i] - 1] != vertex_coords_index[i] - 1)
            {
                printf("No reordering possible.\n");
                printf(
                  "rel1: vt: %d, vc: %d\n",
                  tex_coords_index[i] - 1,
                  vt_ci[tex_coords_index[i] - 1]);
                printf(
                  "rel2: vt: %d, vc: %d\n",
                  tex_coords_index[i] - 1,
                  vertex_coords_index[i] - 1);
                return tex_coords;
            }
            else
                vt_ci[tex_coords_index[i] - 1] = vertex_coords_index[i] - 1;

        for (auto i = 0; i < tex_coords.size(); ++i) re_ordered[i] = tex_coords[vt_ci[i]];

        return re_ordered;
    }

}    // namespace

cr::model_loader::model_data
  cr::model_loader::load_obj(const std::string &file, const std::string &folder)
{
    const auto file_fixed = std::regex_replace(file, std::regex("\\\\"), "/");

    auto model_data = cr::model_loader::model_data();

    auto loader = objl::Loader();

    if (!loader.LoadFile(file_fixed)) cr::exit("Can't open file " + file_fixed);

    for (const auto &mesh : loader.LoadedMeshes)
    {
        //        model_data.vertex_indices.insert(std::end(model_data.vertex_indices),
        //        mesh.Indices.begin(), mesh.Indices.end());
        model_data.texture_indices.insert(
          std::end(model_data.texture_indices),
          mesh.Indices.begin(),
          mesh.Indices.end());

        auto material_data = cr::material::information();
        material_data.name = mesh.MeshMaterial.name;
        material_data.colour =
          glm::vec3(mesh.MeshMaterial.Kd.X, mesh.MeshMaterial.Kd.Y, mesh.MeshMaterial.Kd.Z);
        material_data.type     = cr::material::type::smooth;
        material_data.emission = 0.0f;

        // Texture stuff!
        if (!mesh.MeshMaterial.map_Kd.empty())
        {
            const auto texture_name = folder + '\\' + mesh.MeshMaterial.map_Kd;

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
            std::memcpy(texture_image.data(), data, image_dimensions.x * image_dimensions.y * 4);
            stbi_image_free(data);
            material_data.tex = std::move(texture_image);
        }

        const auto previous_size = model_data.materials.size();
        model_data.materials.emplace_back(material_data);

        for (const auto &vertex : mesh.Vertices)
        {
            model_data.material_indices.push_back(previous_size);

            model_data.vertices.emplace_back(
              vertex.Position.X,
              vertex.Position.Y,
              vertex.Position.Z);

            model_data.texture_coords.emplace_back(
              vertex.TextureCoordinate.X,
              vertex.TextureCoordinate.Y);
        }
    }

    for (auto i = 0; i < model_data.vertices.size(); i++)
    {
        model_data.vertex_indices.push_back(i);
    }
    //    assert(model_data.vertices.size() % 3 == 0 && "Vertex count in model is not a multiple of
    //    3, what the fuck?");

    return model_data;
}

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
            auto data             = stbi_load(
              texture_name.c_str(),
              &image_dimensions.x,
              &image_dimensions.y,
              &image_dimensions.z,
              4);
            //            stbi_set_flip_vertically_on_load(false);

            auto texture_image = cr::image(image_dimensions.x, image_dimensions.y);
            std::memcpy(texture_image.data(), data, image_dimensions.x * image_dimensions.y * 4);
            stbi_image_free(data);
            material_data.tex = std::move(texture_image);
        }

        model_data.materials.emplace_back(material_data);
    }

    for (const auto &shape : shapes)
    {
        for (const auto idx : shape.mesh.indices)
        {
            if (idx.vertex_index != -1) model_data.vertex_indices.push_back(idx.vertex_index);

            if (idx.texcoord_index != -1)
                model_data.texture_indices.push_back(idx.texcoord_index);
        }

        for (int material_id : shape.mesh.material_ids)
        {
            model_data.material_indices.push_back(material_id);
        }
    }

    model_data.texture_coords =
      ::fix(model_data.texture_coords, model_data.vertex_indices, model_data.texture_indices);

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
