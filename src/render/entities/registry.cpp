#include "registry.h"

namespace
{
    template<typename T>
    [[nodiscard]] std::unique_ptr<std::vector<T>> persist(const std::vector<T> &data)
    {
        auto on_heap = std::make_unique<std::vector<T>>(data.size());
        std::memcpy(on_heap.get()->data(), data.data(), sizeof(T) * data.size());
        return std::move(on_heap);
    }
}    // namespace

cr::registry::registry()
{
    entities.prepare<cr::entity::transforms>();
    entities.prepare<cr::entity::model_materials>();
    entities.prepare<cr::entity::model_data>();
    entities.prepare<cr::entity::model_geometry>();
    entities.prepare<cr::entity::light::point>();
    entities.prepare<cr::entity::light::directional>();
    entities.prepare<cr::entity::light::area>();

    // Create the camera
    _camera_entity = entities.create();

    entities.emplace<cr::entity::type>(_camera_entity, cr::entity::type::CAMERA);
    entities.emplace<cr::camera>(_camera_entity);
    entities.emplace<std::string>(_camera_entity, "Camera");
}

cr::raster_objects cr::registry::register_model(const cr::model_loader::model_data &data)
{
    // Expand the data we have have from the indices. Why?
    // Good question - I'm waiting on Intels Embree team to reply to my github issue
    // https://github.com/embree/embree/issues/325

    auto expanded_vertices = std::vector<glm::vec3>();
    expanded_vertices.reserve(data.vertex_indices.size());
    for (const auto index : data.vertex_indices) expanded_vertices.push_back(data.vertices[index]);

    auto expanded_tex_coords = std::vector<glm::vec2>();
    expanded_tex_coords.reserve(data.texture_coords.size());
    for (const auto index : data.texture_indices)
        expanded_tex_coords.push_back(data.texture_coords[index]);

    auto expanded_normals = std::vector<glm::vec3>();
    expanded_normals.reserve(data.normals.size());
    for (const auto index : data.normal_indices) expanded_normals.push_back(data.normals[index]);

    assert(
      expanded_vertices.size() == expanded_tex_coords.size() &&
      "Expanded Vertex Size and Expanded Tex Coord size mismatch");

    auto indices = std::vector<uint32_t>();
    indices.reserve(expanded_vertices.size());
    for (auto i = 0; i < expanded_vertices.size(); i++) indices.push_back(i);

    auto persist_vertices        = ::persist(expanded_vertices);
    auto persist_vertex_indices  = ::persist(indices);
    auto persist_texture_coords  = ::persist(expanded_tex_coords);
    auto persist_texture_indices = ::persist(indices);

    // Create the model embree instance
    auto model_instance = cr::model::instance_geometry(
      *persist_vertices,
      *persist_vertex_indices,
      *persist_texture_coords,
      *persist_texture_indices);

    static auto current_model_count = uint32_t(0);

    auto entity = entities.create();

    entities.emplace<cr::entity::type>(entity, cr::entity::type::MODEL);

    entities.emplace<cr::entity::transforms>(entity);
    entities.get<cr::entity::transforms>(entity).data.emplace_back(glm::mat4(1));

    entities.emplace<cr::entity::model_materials>(entity, data.materials, data.material_indices);

    entities.emplace<cr::entity::model_data>(
      entity,
      std::move(persist_vertices),
      std::move(persist_vertex_indices),
      std::move(persist_texture_coords),
      std::move(persist_texture_indices));

    entities.emplace<cr::entity::model_geometry>(
      entity,
      model_instance.device,
      model_instance.scene,
      model_instance.geometry);
    entities.emplace<std::string>(
      entity,
      std::string("Model - " + std::to_string(++current_model_count)));

    return _get_meshes_by_material(data);
}

cr::raster_objects cr::registry::_get_meshes_by_material(const cr::model_loader::model_data &data)
{
    auto objects = std::vector<cr::temporary_mesh>(data.materials.size());

    for (auto i = 0; i < data.vertex_indices.size(); i++)
    {
        const auto object_index = data.material_indices[i / 3];

        objects[object_index].vertices.push_back(data.vertices[data.vertex_indices[i]]);

        objects[object_index].texture_coords.push_back(
          data.texture_coords[data.texture_indices[i]]);

        objects[object_index].normals.push_back(
          data.normals[data.normal_indices[i]]);
    }

    return _upload_temporary_meshes(objects);
}

cr::raster_objects
  cr::registry::_upload_temporary_meshes(const std::vector<cr::temporary_mesh> &meshes)
{
    auto uploaded = cr::raster_objects();

    for (const auto &mesh : meshes)
    {
        auto new_mesh = cr::mesh(mesh.material);
        // Upload the data

        const auto vertex_data = _zip_mesh_data(mesh);

        glGenVertexArrays(1, &new_mesh.vao);
        glGenBuffers(1, &new_mesh.vbo);
        new_mesh.indices = mesh.vertices.size();

        glBindVertexArray(new_mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, new_mesh.vbo);

        glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), vertex_data.data(), GL_STATIC_DRAW);

        // vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
        // vertex tex coords
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
        // vertex normals
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (5 * sizeof(float)));

        glBindVertexArray(0);

        uploaded.meshes.push_back(std::move(new_mesh));
    }

    return uploaded;
}

std::vector<float> cr::registry::_zip_mesh_data(const cr::temporary_mesh &mesh)
{
    auto data = std::vector<float>(mesh.vertices.size() * 8);

    for (auto i = 0; i < mesh.vertices.size(); i++)
    {
        data[i * 8 + 0] = mesh.vertices[i].x;
        data[i * 8 + 1] = mesh.vertices[i].y;
        data[i * 8 + 2] = mesh.vertices[i].z;
        data[i * 8 + 3] = mesh.texture_coords[i].x;
        data[i * 8 + 4] = mesh.texture_coords[i].y;

        data[i * 8 + 5] = mesh.normals[i].x;
        data[i * 8 + 6] = mesh.normals[i].y;
        data[i * 8 + 7] = mesh.normals[i].z;
    }

    return data;
}

cr::camera *cr::registry::camera()
{
    return &entities.get<cr::camera>(_camera_entity);
}
