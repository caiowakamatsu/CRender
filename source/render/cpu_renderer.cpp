//
// Created by howto on 22/5/2022.
//

#include "cpu_renderer.h"

namespace {

struct ray_extension {
  float cos_theta;
  float pdf;
  glm::vec3 bxdf;
  glm::vec3 emission;
  cr::ray next_ray;
};
[[nodiscard]] std::optional<ray_extension> extend_ray(const cr::ray &ray, std::function<std::optional<cr::intersection>(const cr::ray &)> intersect, cr::random *random) {
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

    return ray_extension {
        .cos_theta = cos_theta,
        .pdf = ray_evaluation.pdf,
        .bxdf = evaluated.bxdf,
        .emission = evaluated.emission,
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

  for (size_t y = data.first.y; y < data.second.y; y++) {
    for (size_t x = data.first.x; x < data.second.x; x++) {
      auto throughput = glm::vec3(1.0f);
      auto accumulated = glm::vec3(0.0f);

      auto ray = data.data->config.ray(x, y, random);

      auto current_extension = ::ray_extension();

      auto had_intersection = false;

      // do primary ray manually
      if (const auto extension_opt = ::extend_ray(ray, data.data->intersect, &random); extension_opt.has_value()) {
        current_extension = extension_opt.value();

        accumulated += current_extension.emission;

        had_intersection = true;
      } else {
        const auto t = ray.direction.y * 0.5f + 0.5f;
        const auto sky = glm::mix(glm::vec3(0.2f, 0.2f, 0.8f), glm::vec3(1.0f), t);

        accumulated = sky;
      }

      if (had_intersection) {
        for (size_t depth = 1; depth < data.data->config.bounces(); depth++) {
          ray = current_extension.next_ray;
          throughput *= current_extension.bxdf * current_extension.cos_theta / current_extension.pdf;

          if (const auto extension_opt = ::extend_ray(ray, data.data->intersect, &random); extension_opt.has_value()) {
            current_extension = extension_opt.value();

            accumulated += throughput * current_extension.emission;
          } else {
            const auto t = ray.direction.y * 0.5f + 0.5f;
            const auto sky = glm::mix(glm::vec3(0.2f, 0.2f, 0.8f), glm::vec3(1.0f), t);

            accumulated += throughput * sky;
            break;
          }
        }
      }



      // Who needs safe math, we have isnan!
      if (!(std::isnan(accumulated.x) || std::isnan(accumulated.y) || std::isnan(accumulated.z))) {
        const auto sample = glm::vec3(data.data->buffer->get(x, y)) * static_cast<float>(data.data->samples);
        const auto averaged = glm::vec4((sample + accumulated) / (data.data->samples + 1.0f), 1.0f);
        data.data->buffer->set(x, y, averaged);
      }
    }
  }
}

cpu_renderer::cpu_renderer() : _pool() {

}

void cpu_renderer::render(const cr::render_data &data, std::span<std::pair<glm::ivec2, glm::ivec2>> tiles) {
  for (const auto &tile : tiles)
  {
    _pool.push_task(_thread_dispatch, thread_render_data {
                                          .random_seed = static_cast<uint64_t>(rand()),
                                          .first = tile.first,
                                          .second = tile.second,
                                          .data = &data,
                                      });
  }
}
void cpu_renderer::wait() {
  _pool.wait_for_tasks();
}
}