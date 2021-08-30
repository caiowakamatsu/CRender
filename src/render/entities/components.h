#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <embree3/rtcore.h>

#include <render/material/material.h>

#include <glad/glad.h>

#include <util/numbers.h>

namespace cr::entity
{
    struct instances
    {
        std::vector<glm::mat4> transforms;
    };

    struct sun
    {
        float     size      = cr::numbers<float>::pi / 48.0f;
        float     intensity = 100.0f;
        glm::vec3 direction = glm::normalize(glm::vec3(0.8, -1, 0.0));
        glm::vec3 colour    = glm::vec3(1.0, 0.9, 0.7);
    };

    struct gpu_data
    {
        GLuint vbo;
        GLuint vao;
        GLuint texture;

        cr::material  material;
        std::uint32_t indices;
    };

    struct model_gpu_data
    {
        std::vector<gpu_data> meshes;
    };

    struct geometry
    {
        geometry() = default;
        explicit geometry(
          std::unique_ptr<std::vector<glm::vec3>> vert_coords,
          std::unique_ptr<std::vector<uint32_t>>  vert_indices,
          std::unique_ptr<std::vector<glm::vec2>> tex_coords)
            : vert_coords(std::move(vert_coords)), vert_indices(std::move(vert_indices)),
              tex_coords(std::move(tex_coords))
        {
        }

        std::unique_ptr<std::vector<glm::vec3>> vert_coords;
        std::unique_ptr<std::vector<uint32_t>>  vert_indices;
        std::unique_ptr<std::vector<glm::vec2>> tex_coords;
    };

    struct embree_ctx
    {
        embree_ctx()
        {
            device   = rtcNewDevice(nullptr);
            scene    = rtcNewScene(device);
            geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
        }

        embree_ctx(RTCDevice device, RTCScene scene, RTCGeometry geometry)
            : device(device), scene(scene), geometry(geometry)
        {
        }

        RTCDevice   device   = nullptr;
        RTCScene    scene    = nullptr;
        RTCGeometry geometry = nullptr;
    };

    struct emissive_triangles
    {
        emissive_triangles() = default;
        explicit emissive_triangles(std::vector<uint32_t> indices)
            : emissive_indices(std::move(indices))
        {
        }
        std::vector<uint32_t> emissive_indices;
    };

    struct model_materials
    {
        model_materials() = default;
        model_materials(
          const std::vector<cr::material> &materials,
          const std::vector<uint32_t> &    indices)
            : materials(materials), indices(indices)
        {
        }

        std::vector<cr::material> materials = {};
        std::vector<uint32_t>     indices   = {};

        std::vector<glm::vec2> tex_coords;
        std::vector<uint32_t>  tex_indices;
    };
}    // namespace cr::entity
