#pragma once

#include <memory>
#include <variant>
#include <type_traits>
#include <algorithm>
#include <random>
#include <cmath>
#include <array>
#include <iostream>
#include <filesystem>

#include <objects/image.h>

#include <render/camera.h>
#include <render/scene.h>
#include <objects/thread_pool.h>
#include <util/sampling.h>

#include <glad/glad.h>

namespace cr
{
    class draft_renderer
    {
    public:
        draft_renderer(
          uint64_t res_x,
          uint64_t res_y,
          std::unique_ptr<cr::scene> *scene);

        GLuint rendered_texture() const;

        void render(uint64_t res_x, uint64_t res_y);

        void update_keyboard_inputs(std::array<char, 300> keys);

    private:
        std::unique_ptr<cr::scene> *_scene = nullptr;

        GLuint _framebuffer = -1;
        GLuint _texture = -1;
        GLuint _depth_buffer = -1;

        uint64_t _res_x;
        uint64_t _res_y;

    };
}
