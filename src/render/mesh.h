#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <vector>

namespace cr
{
    class mesh
    {
    public:
        GLuint vbo;
        GLuint vao;
        GLuint texture;
        cr::material material;
        std::uint32_t indices;

        explicit mesh(const cr::material &material) :
        material(material)
        {
        }

    };

    class temporary_mesh
    {
    public:
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> texture_coords;
        std::vector<glm::vec3> normals;
        cr::material material;
    };

}
