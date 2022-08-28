//
// Created by howto on 22/5/2022.
//
// #include <cmath>

#include "cpu_renderer.h"

namespace {

struct ray_extension {
  float cos_theta;
  float pdf;
  glm::vec3 bxdf;
  glm::vec3 emission;
  glm::vec3 normal;
  glm::vec3 albedo;
  cr::ray next_ray;
};
[[nodiscard]] std::optional<ray_extension> extend_ray(
    const cr::ray &ray,
    std::function<std::optional<cr::intersection>(const cr::ray &)> intersect,
    cr::random *random) {
  if (const auto isect_optional = intersect(ray); isect_optional.has_value()) {
    const auto isect = isect_optional.value();

    const auto point = ray.at(isect.distance);
    const auto material = isect.material;

    const auto ray_evaluation =
        material->evalute_ray(ray.direction, isect.normal, point, random);
    const auto evaluated =
        material->evalute(ray.direction, ray_evaluation.ray.direction,
                          isect.normal, isect.texcoord);
    const auto cos_theta = glm::dot(-ray.direction, isect.normal);
    return ray_extension{
        .cos_theta = cos_theta,
        .pdf = ray_evaluation.pdf,
        .bxdf = evaluated.bxdf,
        .emission = evaluated.emission,
        .normal = isect.normal,
        .albedo = evaluated.albedo,
        .next_ray = ray_evaluation.ray,
    };

  } else {
    return std::nullopt;
  }
}

} // namespace

namespace cr {

void cpu_renderer::_thread_dispatch(thread_render_data data) {
  auto random = cr::random(data.random_seed);

  size_t total_rays = 0;
  for (size_t y = data.first.y; y < data.second.y; y++) {
    for (size_t x = data.first.x; x < data.second.x; x++) {
      auto throughput = glm::vec3(1.0f);
      auto accumulated = glm::vec3(0.0f);

      auto normal = glm::vec3(1.0f);
      auto albedo = glm::vec3(0.0f);

      auto ray = data.data->config.ray(x, y, random);

      auto current_extension = ::ray_extension();

      auto had_intersection = false;

      // do primary ray manually
      if (const auto extension_opt =
              ::extend_ray(ray, data.data->intersect, &random);
          extension_opt.has_value()) {
        current_extension = extension_opt.value();

        accumulated += current_extension.emission;

        had_intersection = true;

        normal = current_extension.normal;
        albedo = current_extension.albedo;
      } else {
        const auto uv =
            glm::vec2(0.5f + std::atan2(ray.direction.z, ray.direction.x) /
                                 (2.0f * glm::pi<float>()),
                      0.5f - std::asin(ray.direction.y) / glm::pi<float>());

        accumulated = skybox.at(uv);
        albedo = skybox.at(uv);
        ;
      }
      total_rays += 1;

      if (had_intersection) {
        for (size_t depth = 1; depth < data.data->config.bounces(); depth++) {
          ray = current_extension.next_ray;
          throughput *= current_extension.bxdf * current_extension.cos_theta /
                        current_extension.pdf;

          total_rays += 1;
          if (const auto extension_opt =
                  ::extend_ray(ray, data.data->intersect, &random);
              extension_opt.has_value()) {
            current_extension = extension_opt.value();

            accumulated += throughput * current_extension.emission;
          } else {
            const auto uv =
                glm::vec2(0.5f + std::atan2(ray.direction.z, ray.direction.x) /
                                     (2.0f * glm::pi<float>()),
                          0.5f - std::asin(ray.direction.y) / glm::pi<float>());

            accumulated += throughput * skybox.at(uv);
            ;
            break;
          }
        }
      }

      if (_sample_count == 0) {
        data.data->normal_buffer->set(x, y, normal);
        data.data->albedo_buffer->set(x, y, albedo);
      }

      // Who needs safe math, we have isnan!
      if (!(std::isnan(accumulated.x) || std::isnan(accumulated.y) ||
            std::isnan(accumulated.z))) {
        const auto sample = glm::vec3(data.data->buffer->get(x, y)) *
                            static_cast<float>(_sample_count.load());
        const auto averaged = glm::vec4(
            (sample + accumulated) / (_sample_count.load() + 1.0f), 1.0f);
        data.data->buffer->set(x, y, averaged);
      }
    }
  }

  _ray_count += total_rays;
}

cpu_renderer::cpu_renderer(int thread_count, component::skybox::Options options,
                           int target_sample_count)
    : _pool(thread_count), skybox(options), _rendering(false), _sample_count(0),
      _ray_count(0), _target_sample_count(target_sample_count) {}

void cpu_renderer::start(render_data data,
                         std::span<std::pair<glm::ivec2, glm::ivec2>> tiles) {
  _rendering = true;
  _sample_count = 0;
  _start_time = std::chrono::high_resolution_clock::now();

  _render_thread = std::thread([this, tiles, data]() {
    while (_rendering) {
      for (const auto &tile : tiles) {
        _pool.push_task([&, this]() {
          _thread_dispatch({
              .random_seed = static_cast<uint64_t>(rand()),
              .first = tile.first,
              .second = tile.second,
              .data = &data,
          });
        });
      }
      _pool.wait_for_tasks();
      _sample_count += 1;
      if (_target_sample_count > 0 && _sample_count >= _target_sample_count) {
        _rendering = false;
      }
    }
  });
}
void cpu_renderer::stop() {
  _rendering = false;
  // Could have finished and reached the target
  if (_render_thread.joinable()) {
    _render_thread.join();
  }
}

size_t cpu_renderer::total_samples() const { return _sample_count; }
double cpu_renderer::total_time() const {
  return std::chrono::duration_cast<std::chrono::duration<double>>(
             std::chrono::high_resolution_clock::now() - _start_time)
      .count();
}

size_t cpu_renderer::total_rays() const { return _ray_count; }

} // namespace cr
