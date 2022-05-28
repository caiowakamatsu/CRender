//
// Created by Caio W on 13/3/22.
//

#ifndef CREBON_GLTF_HPP
#define CREBON_GLTF_HPP

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <fmt/core.h>

#include <tinygltf/tinygltf.h>

#include <util/logger.hpp>

#include <stb/stb_image.h>

#include <optional>
#include <span>

namespace glm {
template <typename T, glm::qualifier Q>
GLM_FUNC_QUALIFIER void compose(glm::mat<4, 4, T, Q> &ModelMatrix,
                                const glm::vec<3, T, Q> &Scale,
                                const glm::qua<T, Q> &Orientation,
                                const glm::vec<3, T, Q> &Translation) {
  ModelMatrix[3] = vec<4, T, Q>(Translation, 1);
  glm::mat<3, 3, T, Q> rotationMat = toMat3(Orientation);
  ModelMatrix[0] = vec<4, T, Q>(rotationMat[0] * Scale[0], 0);
  ModelMatrix[1] = vec<4, T, Q>(rotationMat[1] * Scale[1], 0);
  ModelMatrix[2] = vec<4, T, Q>(rotationMat[2] * Scale[2], 0);
}

template <typename T, glm::qualifier Q>
GLM_FUNC_DECL glm::mat<4, 4, T, Q>
compose(glm::vec<3, T, Q> const &Scale, glm::qua<T, Q> const &Orientation,
        glm::vec<3, T, Q> const &Translation) {
  glm::mat<4, 4, T, Q> ModelMatrix;
  ModelMatrix[3] = vec<4, T, Q>(Translation, 1);
  glm::mat<3, 3, T, Q> rotationMat = glm::toMat3(Orientation);
  ModelMatrix[0] = vec<4, T, Q>(rotationMat[0] * Scale[0], 0);
  ModelMatrix[1] = vec<4, T, Q>(rotationMat[1] * Scale[1], 0);
  ModelMatrix[2] = vec<4, T, Q>(rotationMat[2] * Scale[2], 0);
  return ModelMatrix;
}
} // namespace glm

namespace cr::gltf {
struct loaded_node {
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> texcoords;
  std::vector<uint32_t> indices;
  std::vector<uint32_t> material_indices;
};
[[nodiscard]] loaded_node get_vertices(const tinygltf::Model &model,
                                       const tinygltf::Node &node,
                                       const glm::mat4 &transform,
                                       cr::logger *logger) {
  const auto &mesh = model.meshes[node.mesh];
  logger->info("Loading mesh {}", mesh.name);

  auto joined_vertices = std::vector<glm::vec3>();
  auto joined_normals = std::vector<glm::vec3>();
  auto joined_texcoords = std::vector<glm::vec2>();
  auto joined_indices = std::vector<uint32_t>();
  auto joined_materials = std::vector<uint32_t>();

  for (const auto &primitive : mesh.primitives) {
    auto indices = std::vector<uint16_t>();
    {
      const auto &accessor = model.accessors[primitive.indices];
      const auto &view = model.bufferViews[accessor.bufferView];
      const auto &buffer = model.buffers[view.buffer];

      if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT ||
          accessor.type != TINYGLTF_TYPE_SCALAR) {
        logger->error("Failed to read view {}", view.name);
        exit(-1);
      }

      indices.resize(view.byteLength / sizeof(uint16_t));
      const auto stride = view.byteStride == 0
                              ? sizeof(uint16_t)
                              : view.byteStride; // Is this correct?

      // Todo: add optimised path if the stride is 0, so we can do 1 memcpy
      for (size_t i = 0; i < accessor.count; i++) {
        const auto offset = stride * i;

        auto number = uint16_t();
        std::memcpy(&number, buffer.data.data() + view.byteOffset + offset,
                    sizeof(uint16_t));
        indices[i] = number;
      }
    }

    auto vertices = std::vector<glm::vec3>();
    {
      const auto &it = primitive.attributes.find("POSITION");
      if (it == primitive.attributes.end()) {
        logger->error("Failed to find position attribute");
        exit(-2);
      }
      const auto attribute_idx = it->second;
      const auto &accessor = model.accessors[attribute_idx];
      const auto &view = model.bufferViews[accessor.bufferView];
      const auto &buffer = model.buffers[view.buffer];

      if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
          accessor.type != TINYGLTF_TYPE_VEC3) {
        logger->error("Failed to read view {}", view.name);
        exit(-1);
      }

      vertices.resize(accessor.count);
      const auto stride =
          view.byteStride == 0 ? sizeof(glm::vec3) : view.byteStride;
      for (size_t i = 0; i < accessor.count; i++) {
        const auto offset = stride * i;

        auto vertex = glm::vec3();
        std::memcpy(&vertex, buffer.data.data() + view.byteOffset + offset,
                    sizeof(glm::vec3));
        vertices[i] = vertex;
      }
    }

    auto texcoords = std::vector<glm::vec2>();
    {
      const auto &it = primitive.attributes.find("TEXCOORD_0");
      if (it == primitive.attributes.end()) {
        logger->error("Failed to find texcoord attribute");
        exit(-2);
      }
      const auto attribute_idx = it->second;
      const auto &accessor = model.accessors[attribute_idx];
      const auto &view = model.bufferViews[accessor.bufferView];
      const auto &buffer = model.buffers[view.buffer];

      if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
          accessor.type != TINYGLTF_TYPE_VEC2) {
        logger->error("Failed to read view {}", view.name);
        exit(-1);
      }

      texcoords.resize(accessor.count);
      const auto stride =
          view.byteStride == 0 ? sizeof(glm::vec2) : view.byteStride;
      for (size_t i = 0; i < accessor.count; i++) {
        const auto offset = stride * i;

        auto texcoord = glm::vec2();
        std::memcpy(&texcoord, buffer.data.data() + view.byteOffset + offset,
                    sizeof(glm::vec2));
        texcoords[i] = texcoord;
      }
    }

    auto normals = std::vector<glm::vec3>();
    {
      const auto &it = primitive.attributes.find("NORMAL");
      if (it == primitive.attributes.end()) {
        logger->error("Failed to find normal attribute");
        exit(-2);
      }
      const auto attribute_idx = it->second;
      const auto &accessor = model.accessors[attribute_idx];
      const auto &view = model.bufferViews[accessor.bufferView];
      const auto &buffer = model.buffers[view.buffer];

      if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
          accessor.type != TINYGLTF_TYPE_VEC3) {
        logger->error("Failed to read view {}", view.name);
        exit(-1);
      }

      normals.resize(accessor.count);
      const auto stride =
          view.byteStride == 0 ? sizeof(glm::vec3) : view.byteStride;
      for (size_t i = 0; i < accessor.count; i++) {
        const auto offset = stride * i;

        auto normal = glm::vec3();
        std::memcpy(&normal, buffer.data.data() + view.byteOffset + offset,
                    sizeof(glm::vec3));
        normals[i] = normal;
      }
    }

    joined_indices.reserve(joined_vertices.size() + indices.size());
    for (auto idx : indices)
      joined_indices.push_back(joined_vertices.size() + idx);

    joined_normals.reserve(joined_normals.size() + normals.size());
    for (auto normal : normals)
      joined_normals.push_back(normal);

    for (auto &vertex : vertices)
      vertex = glm::vec3(transform * glm::vec4(vertex, 1.0f));

    joined_vertices.reserve(joined_vertices.size() + vertices.size());
    for (auto vertex : vertices)
      joined_vertices.push_back(vertex);

    joined_materials.reserve(joined_materials.size() + indices.size());
    for (size_t i = 0; i < indices.size(); i++)
      joined_materials.push_back(primitive.material);

    joined_texcoords.reserve(joined_texcoords.size() + texcoords.size());
    for (auto texcoord : texcoords)
      joined_texcoords.push_back(texcoord);
  }

  return {.vertices = joined_vertices,
          .normals = joined_normals,
          .texcoords = joined_texcoords,
          .indices = joined_indices,
          .material_indices = joined_materials};
}

[[nodiscard]] glm::mat4 get_transformation(const tinygltf::Node &node) {
  auto transform = glm::mat4();

  if (auto &mat = node.matrix; mat.size() == 16)
    transform = {
        mat[0], mat[1], mat[2],  mat[3],  mat[4],  mat[5],  mat[6],  mat[7],
        mat[8], mat[9], mat[10], mat[11], mat[12], mat[13], mat[14], mat[15],
    };
  else {
    auto pos = glm::vec3{0.f, 0.f, 0.f};
    auto rot = glm::quat(1.f, 0.f, 0.f, 0.f);
    auto scale = glm::vec3{1.f, 1.f, 1.f};

    if (node.scale.size() == 3)
      for (auto i = 0; i < 3; i++)
        scale[i] = static_cast<float>(node.scale[i]);

    if (node.rotation.size() == 4)
      for (auto i = 0; i < 4; i++)
        rot[i] = static_cast<float>(node.rotation[i]);

    if (node.translation.size() == 3)
      for (auto i = 0; i < 3; i++)
        pos[i] = static_cast<float>(node.translation[i]);

    return glm::compose(scale, rot, pos);
  }

  return transform;
}

[[nodiscard]] std::optional<glm::vec3>
material_albedo(tinygltf::ParameterMap *additionalValues) {
  const auto as_base_colour = additionalValues->find("baseColorFactor");
  if (as_base_colour != additionalValues->end()) {
    const auto &parameter = as_base_colour->second;
    const auto &colour = parameter.ColorFactor();
    return glm::vec3(colour[0], colour[1], colour[2]);
  }

  // Todo: add more ways for checking base colour (are there any other
  // conventions?)

  return std::nullopt;
}

[[nodiscard]] std::optional<cr::image> material_albedo_texture(
    tinygltf::Material *material, std::span<tinygltf::Texture> textures,
    std::span<tinygltf::Image> images, std::span<tinygltf::Sampler> samplers, cr::logger *logger) {

  if (material->pbrMetallicRoughness.baseColorTexture.index != -1) {
    const auto texture =
        textures[material->pbrMetallicRoughness.baseColorTexture.index];
    const auto image = images[texture.source];
    // Todo: use sampler somehow...?

    // Check if PNG
    if (image.mimeType.find("png") != std::string::npos) {

      if (image.component != 4) {
        logger->error("Base colour texture is not RGBA");
        return std::nullopt;
      }

      auto as_floats = std::vector<float>(image.width * image.height * 4);
      for (size_t i = 0; i < image.image.size(); i++) {
        as_floats[i] = image.image[i] / 255.0f;
      }

      return cr::image(as_floats, image.width, image.height);
    }
  }

  return std::nullopt;
}

[[nodiscard]] std::optional<cr::image> material_emissive_texture(
    tinygltf::Material *material, std::span<tinygltf::Texture> textures,
    std::span<tinygltf::Image> images, std::span<tinygltf::Sampler> samplers, cr::logger *logger) {

  if (material->emissiveTexture.index != -1) {
    const auto texture = textures[material->emissiveTexture.index];
    const auto image = images[texture.source];
    // Todo: use sampler somehow...?
    //			[[maybe_unused]] const auto sampler =
    //samplers[texture.sampler];

    // Check if PNG
    if (image.mimeType.find("png") != std::string::npos) {

      if (image.component != 4) {
        logger->error("failed to load png image should have 4 channels");
        return std::nullopt;
      }

      auto as_floats = std::vector<float>(image.width * image.height * 4);
      for (size_t i = 0; i < image.image.size(); i++) {
        as_floats[i] = image.image[i] / 255.0f;
      }

      return cr::image(as_floats, image.width, image.height);
    }
  }

  return std::nullopt;
}

[[nodiscard]] std::optional<glm::vec3>
material_emission(const std::vector<double> &emissive) {
  if (emissive.size() == 3) {
    return glm::vec3(emissive[0], emissive[1], emissive[2]);
  } else {
    return std::nullopt;
  }
}

enum class material_type_e { mirror, glass, metal, diffuse };

[[nodiscard]] std::optional<float>
material_roughness(tinygltf::Material *material) {
  auto values = material->values;
  auto it = values.find("roughnessFactor");
  if (it != values.end())
    return it->second.Factor();
  return std::nullopt;
}

[[nodiscard]] std::optional<cr::image> material_roughness_texture(
    tinygltf::Material *material, std::span<tinygltf::Texture> textures,
    std::span<tinygltf::Image> images, std::span<tinygltf::Sampler> samplers, cr::logger *logger) {

  if (material->pbrMetallicRoughness.metallicRoughnessTexture.index != -1) {
    const auto texture =
        textures[material->pbrMetallicRoughness.metallicRoughnessTexture.index];
    const auto image = images[texture.source];
    // Todo: use sampler somehow...?
    if (image.mimeType.find("png") != std::string::npos) {
      if (image.component != 4) {
        logger->error("failed to load png image should have 4 channels\n");
        return std::nullopt;
      }

      auto as_floats = std::vector<float>(image.width * image.height * 4);
      for (size_t i = 0; i < image.image.size(); i++) {
        const auto channel = i % 4;
        const auto pixel_roughness = image.image[((i / 4) * 4) + 1];
        as_floats[i] = pixel_roughness;
      }

      return cr::image(as_floats, image.width, image.height);
    }
  }

  return std::nullopt;
}

[[nodiscard]] std::optional<float>
material_metalness(tinygltf::Material *material) {
  auto values = material->values;
  auto it = values.find("metallicFactor");
  if (it != values.end())
    return it->second.Factor();
  return std::nullopt;
}

[[nodiscard]] std::optional<cr::image> material_metalness_texture(
    tinygltf::Material *material, std::span<tinygltf::Texture> textures,
    std::span<tinygltf::Image> images, std::span<tinygltf::Sampler> samplers, cr::logger *logger) {

  if (material->pbrMetallicRoughness.metallicRoughnessTexture.index != -1) {
    const auto texture =
        textures[material->pbrMetallicRoughness.metallicRoughnessTexture.index];
    const auto image = images[texture.source];
    // Todo: use sampler somehow...?
    if (image.mimeType.find("png") != std::string::npos) {
      if (image.component != 4) {
        logger->error("failed to load png image should have 4 channels\n");
        return std::nullopt;
      }

      auto as_floats = std::vector<float>(image.width * image.height * 4);
      for (size_t i = 0; i < image.image.size(); i++) {
        const auto channel = i % 4;
        const auto pixel_roughness = image.image[((i / 4) * 4) + 2];
        as_floats[i] = pixel_roughness;
      }

      return cr::image(as_floats, image.width, image.height);
    }
  }

  return std::nullopt;
}

[[nodiscard]] material_type_e material_type(tinygltf::Material *material) {
  return material_type_e::diffuse;
}

[[nodiscard]] std::unique_ptr<cr::material> load_material(
    tinygltf::Material *material, std::span<tinygltf::Texture> textures,
    std::span<tinygltf::Image> images, std::span<tinygltf::Sampler> samplers, cr::logger *logger) {
  // 0.299 ∙ Red + 0.587 ∙ Green + 0.114 ∙ Blue
  auto loaded = std::unique_ptr<cr::material>();

  const auto roughness_scaler = material_roughness(material);
  const auto roughness_texture =
      material_roughness_texture(material, textures, images, samplers, logger);
  const auto roughness = [&]() {
    if (roughness_scaler.has_value())
      return cr::sampleable<float>(roughness_scaler.value());
    else if (roughness_texture.has_value())
      return cr::sampleable<float>(roughness_texture.value());
    else {
      logger->warning("no roughness found for material {}", material->name);
      return cr::sampleable<float>(0.0f);
    }
  }();

  const auto metalness_scaler = material_metalness(material);
  const auto metalness_texture =
      material_metalness_texture(material, textures, images, samplers, logger);
  const auto metalness = [&]() {
    if (metalness_scaler.has_value())
      return cr::sampleable<float>(metalness_scaler.value());
    else if (metalness_texture.has_value())
      return cr::sampleable<float>(metalness_texture.value());
    else {
      logger->warning("no metalness found for material {}", material->name);
      return cr::sampleable<float>(0.0f);
    }
  }();

  const auto emission_scalers = material_emission(material->emissiveFactor);
  const auto emissive_texture =
      material_emissive_texture(material, textures, images, samplers, logger);
  const auto emission = [&]() {
    if (emission_scalers.has_value())
      return cr::sampleable<glm::vec3>(emission_scalers.value());
    else if (emissive_texture.has_value())
      return cr::sampleable<glm::vec3>(emissive_texture.value());
    else {
      logger->warning("no emission found for material {}", material->name);
      return cr::sampleable<glm::vec3>(glm::vec3(0.0f));
    }
  }();

  const auto base_colour_scalers = material_albedo(&material->additionalValues);
  const auto base_colour_texture =
      material_albedo_texture(material, textures, images, samplers, logger);
  const auto base_colour = [&]() {
    if (base_colour_scalers.has_value())
      return cr::sampleable<glm::vec3>(base_colour_scalers.value());
    else if (base_colour_texture.has_value())
      return cr::sampleable<glm::vec3>(base_colour_texture.value());
    else {
      logger->warning("no base colour found for material {}", material->name);
      return cr::sampleable<glm::vec3>(glm::vec3(0.0f, 0.0f, 0.0f));
    }
  }();

  loaded = std::make_unique<cr::gltf_material>(roughness, metalness, emission,
                                               base_colour);

  return loaded;
}
} // namespace cr::gltf

#endif // CREBON_GLTF_HPP
