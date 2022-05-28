//
// Created by howto on 22/5/2022.
//

#ifndef CREBON_CPU_RENDERER_H
#define CREBON_CPU_RENDERER_H

#include <render/sky.hpp>
#include <scene/scene.hpp>
#include <util/atomic_image.hpp>

#include <thread-pool/thread_pool.hpp>

#include <span>
#include <atomic>

namespace cr {
struct render_data {
  cr::atomic_image *buffer;
  std::function<std::optional<cr::intersection>(const cr::ray &)> intersect;
  const cr::scene_configuration config;
};

class cpu_renderer {
private:
  std::atomic<bool> _rendering;
  std::atomic<size_t> _sample_count;
  std::thread _render_thread;
  thread_pool _pool;

  int _target_sample_count;

  struct thread_render_data {
    const uint64_t random_seed;
    const glm::ivec2 first;
    const glm::ivec2 second;
    const render_data *const data;
  };
  void _thread_dispatch(thread_render_data data);

public:
  sky sky;

  explicit cpu_renderer(int thread_count, component::skybox::Options options, int target_sample_count);

  void start(cr::render_data data,
             std::span<std::pair<glm::ivec2, glm::ivec2>> tiles);

  void stop();
};
} // namespace cr

#endif // CREBON_CPU_RENDERER_H
