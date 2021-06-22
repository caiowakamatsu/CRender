#pragma once

#include <memory>
#include <variant>
#include <type_traits>
#include <sstream>
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
#include <util/logger.h>

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

        void render();

    private:
        std::unique_ptr<cr::scene> *_scene = nullptr;

        GLuint _framebuffer = -1;
        GLuint _texture = -1;
        GLuint _depth_buffer = -1;
        GLuint _rbo;

        GLuint _vertex_handle;
        GLuint _fragment_handle;
        GLuint _program_handle;

        GLuint _vbo;
        GLuint _vao;

        uint64_t _res_x;
        uint64_t _res_y;

        void _update_uniforms();

    };
}
