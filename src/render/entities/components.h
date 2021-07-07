#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <embree3/rtcore.h>

#include <render/material/material.h>
#include <render/entities/entity_type.h>

#include <util/numbers.h>

namespace cr::entity
{
    struct transforms
    {
        std::vector<glm::mat4> data = {};
    };

    struct sun
    {
        float size = cr::numbers<float>::pi / 48.0f;
        float intensity = 100.0f;
        glm::vec3 direction = glm::normalize(glm::vec3(0.8, -1, 0.0));
        glm::vec3 colour = glm::vec3(1.0, 0.9, 0.7);
    };

    struct model_materials
    {
        model_materials() = default;
        model_materials(
          const std::vector<cr::material> &       materials,
          const std::vector<uint32_t> &           indices)
            : materials(materials), indices(indices)
        {
        }

        std::vector<cr::material> materials = {};
        std::vector<uint32_t>     indices   = {};

        std::vector<glm::vec2> tex_coords;
        std::vector<uint32_t> tex_indices;
    };

    struct model_data
    {
        model_data() = default;
        explicit model_data(
          std::unique_ptr<std::vector<glm::vec3>> vert_coords,
          std::unique_ptr<std::vector<uint32_t>> vert_indices,
          std::unique_ptr<std::vector<glm::vec2>> tex_coords,
          std::unique_ptr<std::vector<uint32_t>> tex_indices)
        :
        vert_coords(std::move(vert_coords)), vert_indices(std::move(vert_indices)),
            tex_coords(std::move(tex_coords)), tex_indices(std::move(tex_indices))
        {
        }

        std::unique_ptr<std::vector<glm::vec3>> vert_coords;
        std::unique_ptr<std::vector<uint32_t>> vert_indices;
        std::unique_ptr<std::vector<glm::vec2>> tex_coords;
        std::unique_ptr<std::vector<uint32_t>> tex_indices;
    };

    struct model_geometry
    {
        model_geometry()
        {
            device   = rtcNewDevice(nullptr);
            scene    = rtcNewScene(device);
            geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
        }

        model_geometry(RTCDevice device, RTCScene scene, RTCGeometry geometry)
            : device(device), scene(scene), geometry(geometry)
        {
        }

        RTCDevice   device   = nullptr;
        RTCScene    scene    = nullptr;
        RTCGeometry geometry = nullptr;
    };

    namespace light
    {
        struct point
        {
            float     intensity;
            glm::vec3 colour;
            glm::vec3 position;
        };

        struct directional
        {
            directional(const glm::vec3 &colour, const glm::vec3 &direction, float intensity)
                : colour(colour), direction(direction), intensity(intensity)
            {
            }

            glm::vec3 colour;
            glm::vec3 direction;
            float     intensity;
        };

        struct area
        {
            area(
              const glm::vec3 &colour,
              const glm::vec3 &position,
              const glm::vec2 &size,
              float            intensity)
                : colour(colour), position(position), size(size), matrix(1), intensity(intensity)
            {
            }

            void recalc_mat()
            {
                matrix = glm::mat4(1);

                matrix = glm::translate(matrix, position);
                matrix = glm::scale(matrix, glm::vec3(size.x, 0, size.y));
            }

            glm::vec2 size;
            glm::vec3 colour;
            glm::vec3 position;
            glm::mat4 matrix;
            float     intensity;
        };

    }    // namespace light

}    // namespace cr::entity
