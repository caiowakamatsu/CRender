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
    class renderer
    {
    public:
        renderer(
          const uint64_t                    res_x,
          const uint64_t                    res_y,
          const uint64_t                    bounces,
          std::unique_ptr<cr::thread_pool> *pool,
          std::unique_ptr<cr::scene> *      scene);

        ~renderer();

        void start();

        void pause();

        void update(const std::function<void()> &update);

        void set_resolution(int x, int y);

        void set_max_bounces(int bounces);

        [[nodiscard]] cr::image *current_progress() noexcept;

    private:
        [[nodiscard]] std::vector<std::function<void()>> _get_tasks();

        void _sample_pixel(uint64_t x, uint64_t y);

        cr::camera *_camera;
        uint64_t    _res_x;
        uint64_t    _res_y;
        float _aspect_correction = 1;
        uint64_t    _max_bounces;

        std::unique_ptr<cr::thread_pool> *_thread_pool;
        std::unique_ptr<cr::scene> *      _scene;

        std::vector<float> _raw_buffer;

        cr::image _buffer;

        std::atomic<bool>     _run_management = true;
        std::atomic<bool>     _pause          = false;
        std::atomic<uint64_t> _current_sample = 0;
        std::thread           _management_thread;

        std::mutex              _start_mutex;
        std::condition_variable _start_cond_var;

        std::mutex              _pause_mutex;
        std::condition_variable _pause_cond_var;
    };
}    // namespace cr