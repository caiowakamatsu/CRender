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

        struct bloom_settings
        {
            bool enabled = false;
            float threshold = 0.7f;
            float strength = 1.0f;
        };
        void submit_bloom_settings(const bloom_settings &settings);

        struct gray_scale_settings
        {
            bool enabled = false;
        };
        void submit_gray_scale_settings(const gray_scale_settings &settings);

        struct tonemapping_settings
        {
            bool enabled = false;
            int type = 0;
            float exposure = 1.f;
            float gamma_correction = 2.2f;
        };
        void submit_tonemapping_settings(const tonemapping_settings &settings);

    private:
        [[nodiscard]] cr::image _blur(const cr::image &source, const glm::ivec2 &dimensions) const noexcept;

        [[nodiscard]] cr::image _brightness(const cr::image &source) const noexcept;

        bloom_settings _bloom_settings;

        gray_scale_settings _gray_scale_settings;

        tonemapping_settings _tonemapping_settings;

        struct
        {
            GLuint compute_program;
            GLuint compute_shader;

            GLuint blur_program;
            GLuint blur_shader;

        } _gpu_handles;
    };
}