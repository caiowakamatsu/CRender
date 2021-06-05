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

namespace cr
{
    class draft_renderer
    {
    public:
        draft_renderer(
          const uint64_t res_x,
          const uint64_t res_y,
          std::unique_ptr<cr::scene> *scene);

        void render_to_framebuffer(uint32_t target_framebuffer, uint64_t res_x, uint64_t res_y);

        void update_keyboard_inputs(std::array<char, 300> keys);

    private:


    };
}
