#pragma once

#include <glad/glad.h>

namespace cr
{
    class mesh
    {
    public:
        GLuint vbo;
        GLuint vao;
        std::uint32_t indices;
        // Used to grab the material data when uploading the mesh to the GPU
        std::uint32_t model_handle;
    };
}
