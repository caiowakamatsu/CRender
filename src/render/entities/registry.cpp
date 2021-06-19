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

cr::mesh cr::registry::register_model(const cr::model_loader::model_data &data)
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
    for (const auto index : data.normal_indices)
        expanded_normals.push_back(data.normals[index]);

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

    // Combine the vertex positions and uv coordinates such as
    // [[x, y, z] [u, v], [nx, ny, nz]
    auto vertex_data = std::vector<float>(expanded_vertices.size() * 8);

    for (auto i = 0; i < expanded_vertices.size(); i++)
    {
        vertex_data[i * 8 + 0] = expanded_vertices[i].x;
        vertex_data[i * 8 + 1] = expanded_vertices[i].y;
        vertex_data[i * 8 + 2] = expanded_vertices[i].z;
        vertex_data[i * 8 + 3] = expanded_tex_coords[i].x;
        vertex_data[i * 8 + 4] = expanded_tex_coords[i].y;

        vertex_data[i * 8 + 5] = expanded_normals[i].x;
        vertex_data[i * 8 + 6] = expanded_normals[i].y;
        vertex_data[i * 8 + 7] = expanded_normals[i].z;
    }

    auto mesh = cr::mesh();

    // do texture stuff!
    glGenTextures(1, &mesh.texture);
    glBindTexture(GL_TEXTURE_2D, mesh.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data.

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    mesh.indices = expanded_vertices.size();

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

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
    return mesh;
}

cr::camera *cr::registry::camera()
{
    return &entities.get<cr::camera>(_camera_entity);
}
