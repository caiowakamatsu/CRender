//
// Created by howto on 22/5/2022.
//

#ifndef CREBON_CPU_RENDERER_H
#define CREBON_CPU_RENDERER_H

#include <util/atomic_image.hpp>
#include <scene/scene.hpp>

#include <thread-pool/thread_pool.hpp>

#include <span>

namespace cr {
  struct render_data {
    const size_t samples;
    cr::atomic_image *buffer;
    std::function<std::optional<cr::intersection>(const cr::ray &)> intersect;
    const cr::scene_configuration config;
  };

  class cpu_renderer {
  private:
    thread_pool _pool;

    struct thread_render_data {
      const uint64_t random_seed;
      const glm::ivec2 first;
      const glm::ivec2 second;
      const render_data * const data;
    };
    static void _thread_dispatch(thread_render_data data);

  public:
    cpu_renderer();

    void render(const cr::render_data& data, std::span<std::pair<glm::ivec2, glm::ivec2>> tiles);

    void wait();
  };
}

#endif // CREBON_CPU_RENDERER_H
