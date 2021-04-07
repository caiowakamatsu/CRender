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
    class new_renderer
    {
    public:
        new_renderer(
          const cr::camera &camera,
          const uint64_t    res_x,
          const uint64_t    res_y,
          cr::thread_pool * pool,
          cr::new_scene *   scene);

        ~new_renderer();

        void start();

        void pause();

        [[nodiscard]] cr::image *current_progress() noexcept;

        cr::thread_pool *thread_pool;
        cr::new_scene *  scene;

    private:
        [[nodiscard]] std::vector<std::function<void()>> _get_tasks();

        void _sample_pixel(uint64_t x, uint64_t y);

        const cr::camera _camera;
        const uint64_t   _res_x;
        const uint64_t   _res_y;

        cr::image _buffer;

        std::atomic<bool>     _run_management;
        std::atomic<bool>     _pause;
        std::atomic<uint64_t> _current_sample = 0;
        std::thread           _management_thread;

        std::mutex              _start_mutex;
        std::condition_variable _start_cond_var;
    };

    class renderer
    {
    public:
        struct config
        {
            enum mode
            {
                TILE,
                LINE,
            };
            mode render_mode = LINE;
            enum termination_strategy
            {
                //                RUSSIAN_ROULETTE,
                NUMERICAL_CUTOFF,
                ATTENUATION_CUTOFF
            };
            termination_strategy termination_strategy = NUMERICAL_CUTOFF;
            union
            {
                uint32_t max_bounce;
                //                float attenuation_cutoff;
            } termination_val;
            struct
            {
                uint32_t x = 512;
                uint32_t y = 512;
            } resolution;
            cr::camera camera;
            uint16_t   thread_count = 16;
        };

        explicit renderer(cr::renderer::config config);

        ~renderer();

        void start();

        void stop();

        void attach_scene(std::unique_ptr<cr::scene> scene);

        [[nodiscard]] cr::renderer::config current_config() const noexcept;

        [[nodiscard]] cr::scene *scene() const noexcept;

        [[nodiscard]] cr::image current_progress();

        [[nodiscard]] std::unique_ptr<cr::renderer> from_config(config config);

    private:
        [[nodiscard]] bool _should_render();

        [[nodiscard]] std::vector<std::function<void()>> _get_tasks();

        void _sample_pixel(uint64_t x, uint64_t y);

        struct _processed_hit
        {
            float     emission;
            float     reflectiveness;
            cr::ray   ray;
            glm::vec3 albedo;
        };
        [[nodiscard]] _processed_hit _process_hit(
          const cr::ray &                       ray,
          const cr::scene::intersection_record &record) const noexcept;

        [[nodiscard]] static float _rand() noexcept;

        cr::renderer::config _config;

        bool     _should_render_val    = true;
        uint64_t _current_sample_count = 0;

        std::mutex _should_render_mutex;
        std::mutex _buffer_mutex;

        cr::image _current_buffer;

        cr::thread_pool _thread_pool;

        std::unique_ptr<cr::scene> _scene;

        std::thread _render_thread;
    };
}    // namespace cr