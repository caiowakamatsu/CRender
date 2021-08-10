#pragma once

#include <objects/image.h>
#include <util/asset_loader.h>
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

        void enable_bloom();

        void enable_gray_scale();

        void disable_bloom();

        void disable_gray_scale();

    private:
        [[nodiscard]] cr::image _blur(const cr::image &source, const glm::ivec2 &dimensions) const noexcept;

        [[nodiscard]] cr::image _brightness(const cr::image &source, float brightness_required) const noexcept;

        bool _use_bloom = true;
        bool _use_gray_scale = false;

        struct
        {
            GLuint compute_program;
            GLuint compute_shader;

            GLuint blur_program;
            GLuint blur_shader;

        } _gpu_handles;
    };
}