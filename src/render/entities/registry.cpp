#include "registry.h"

namespace
{
    template<typename T>
    [[nodiscard]] std::unique_ptr<std::vector<T>> persist(const std::vector<T> &data)
    {
        auto on_heap = std::make_unique<std::vector<T>>(data.size());
        std::memcpy(on_heap->data(), data.data(), sizeof(T) * data.size());
        return std::move(on_heap);
    }

    template<typename T>
    [[nodiscard]] std::unique_ptr<std::vector<T>>
      persist_and_expand(const std::vector<T> &data, const std::vector<uint32_t> &indices)
    {
        assert(indices.size() > 0 && "Cannot expand data with 0 indices");

        auto on_heap = std::make_unique<std::vector<T>>(indices.size());

        for (auto i = 0; i < indices.size(); i++) (*on_heap)[i] = data[indices[i]];

        return on_heap;
    }

}    // namespace

cr::registry::registry()
{
    entities.prepare<std::string>();

    entities.prepare<cr::image>();
    entities.prepare<cr::entity::instances>();
    entities.prepare<cr::entity::gpu_data>();
    entities.prepare<cr::entity::geometry>();
    entities.prepare<cr::entity::embree_ctx>();
    entities.prepare<cr::entity::model_materials>();

    // Create the camera
    _camera_entity = entities.create();

    entities.emplace<cr::camera>(_camera_entity);
    entities.emplace<std::string>(_camera_entity, "Camera");
    auto sun_dir_local_coords = cr::sampling::build_local(-_sun.direction);
    _sun_transform            = glm::mat3(
      sun_dir_local_coords.tangent,
      sun_dir_local_coords.normal,
      sun_dir_local_coords.bi_tangent);
}

void cr::registry::register_model(const cr::asset_loader::model_data &data)
{
    // Expand the data we have have from the indices. Why?
    // Good question - I'm waiting on Intels Embree team to reply to my github issue - And give a
    // helpful answer
    // https://github.com/embree/embree/issues/325
    _upload_gpu_meshes(data);

    auto indices = std::make_unique<std::vector<uint32_t>>(data.vertex_indices.size());
    std::generate(indices->begin(), indices->end(), [n = 0]() mutable { return n++; });

    auto vertices       = ::persist_and_expand(data.vertices, data.vertex_indices);
    auto texture_coords = ::persist_and_expand(data.texture_coords, data.texture_indices);

    static auto current_model_count = uint32_t(0);
    auto        entity              = entities.create();

    // Create the model embree instance
    auto model_instance = cr::model::instance_geometry(*vertices, *indices, *texture_coords);

    auto instances = std::vector<glm::mat4>(1);
    instances[0]   = glm::mat4(1);

    // Translate the materials texture handle from model data to the ECS entity
    auto ecs_handles = std::vector<uint32_t>(data.textures.size());
    for(auto i = 0; i < data.textures.size(); i++)
    {
        ecs_handles[i] = entities.create();
        entities.emplace<cr::image>(ecs_handles[i], data.textures[i]);
    }

    auto updated_materials = std::vector<cr::material>(data.materials.size());
    for (auto i = 0; i < data.materials.size(); i++)
    {
        updated_materials[i] = data.materials[i];
        updated_materials[i].info.tex = ecs_handles[data.materials[i].info.tex.value()];
    }

    entities.emplace<cr::entity::model_materials>(entity, updated_materials, data.material_indices);
    entities.emplace<cr::entity::geometry>(entity, std::move(vertices), std::move(indices), std::move(texture_coords));
    entities.emplace<cr::entity::embree_ctx>(entity, model_instance);
    entities.emplace<cr::entity::instances>(entity, instances);
    entities.emplace<std::string>(entity, data.name);
}

void cr::registry::_upload_gpu_meshes(const cr::asset_loader::model_data &data)
{
    struct mesh
    {
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        cr::material material;
    };
    auto meshes = std::vector<mesh>(data.materials.size());

    for (auto i = 0; i < meshes.size(); i++)
        meshes[i].material = data.materials[i];

    for (auto i = 0; i < data.vertex_indices.size(); i++)
    {
        const auto mesh_index = data.material_indices[i / 3];

        meshes[mesh_index].vertices.push_back(data.vertices[data.vertex_indices[i]]);
        meshes[mesh_index].normals.push_back(data.normals[data.normal_indices[i]]);
        meshes[mesh_index].uvs.push_back(data.texture_coords[data.texture_indices[i]]);
    }

    for (const auto &mesh : meshes)
    {
        auto gpu = cr::entity::gpu_data();

        const auto vertex_data = _zip_mesh_data(mesh.vertices, mesh.normals, mesh.uvs);

        // Upload the mesh to the GPU
        if (mesh.material.info.tex.has_value())
        {
            glGenTextures(1, &gpu.texture);
            glBindTexture(GL_TEXTURE_2D, gpu.texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            const auto &texture = data.textures[mesh.material.info.tex.value()];
            glTexImage2D(
              GL_TEXTURE_2D,
              0,
              GL_RGBA,
              texture.width(),
              texture.height(),
              0,
              GL_RGBA,
              GL_FLOAT,
              texture.data());
        }

        glGenVertexArrays(1, &gpu.vao);
        glGenBuffers(1, &gpu.vbo);
        gpu.indices = mesh.vertices.size();

        glBindVertexArray(gpu.vao);
        glBindBuffer(GL_ARRAY_BUFFER, gpu.vbo);

        glBufferData(
          GL_ARRAY_BUFFER,
          vertex_data.size() * sizeof(float),
          vertex_data.data(),
          GL_STATIC_DRAW);

        // vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
        // vertex tex coords
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
          1,
          2,
          GL_FLOAT,
          GL_FALSE,
          8 * sizeof(float),
          (void *) (3 * sizeof(float)));
        // vertex normals
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
          2,
          3,
          GL_FLOAT,
          GL_FALSE,
          8 * sizeof(float),
          (void *) (5 * sizeof(float)));

        glBindVertexArray(0);

        gpu.material = std::move(mesh.material);

        auto entity = entities.create();
        entities.emplace<cr::entity::gpu_data>(entity, gpu);
    }
}

std::vector<float> cr::registry::_zip_mesh_data(
  const std::vector<glm::vec3> &vertices,
  const std::vector<glm::vec3> &normals,
  const std::vector<glm::vec2> &texture_coords)
{
    auto data = std::vector<float>(vertices.size() * 8);

    for (auto i = 0; i < vertices.size(); i++)
    {
        data[i * 8 + 0] = vertices[i].x;
        data[i * 8 + 1] = vertices[i].y;
        data[i * 8 + 2] = vertices[i].z;
        data[i * 8 + 3] = texture_coords[i].x;
        data[i * 8 + 4] = texture_coords[i].y;

        data[i * 8 + 5] = normals[i].x;
        data[i * 8 + 6] = normals[i].y;
        data[i * 8 + 7] = normals[i].z;
    }

    return data;
}

cr::camera *cr::registry::camera()
{
    return &entities.get<cr::camera>(_camera_entity);
}
cr::entity::sun cr::registry::sun()
{
    return _sun;
}

glm::mat3 cr::registry::sun_transform()
{
    return _sun_transform;
}

void cr::registry::set_sun(const cr::entity::sun &sun)
{
    _sun                      = sun;
    auto sun_dir_local_coords = cr::sampling::build_local(-sun.direction);
    _sun_transform            = glm::mat3(
      sun_dir_local_coords.tangent,
      sun_dir_local_coords.normal,
      sun_dir_local_coords.bi_tangent);
}
