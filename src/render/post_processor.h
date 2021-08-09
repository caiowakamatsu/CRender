#pragma once

#include <objects/image.h>
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <array>

namespace cr
{
    class post_processor
    {
    public:
        post_processor();

        [[nodiscard]] cr::image process(const cr::image &image) const noexcept;
    private:
        struct
        {
            GLuint compute_program;
            GLuint compute_shader;
        } _gpu_handles;
    };
}