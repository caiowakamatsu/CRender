#include <render/cpu_renderer.h>

#include <scene/scene.hpp>
#include <scene/sphere_scene.hpp>
#include <scene/triangular_scene.hpp>

#include <ui/display.hpp>

#include <fmt/core.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include <tinyobjloader/tinyobjloader.h>

#define TINYGLTF_IMPLEMENTATION

#include <tinygltf/tinygltf.h>

#define CGLTF_IMPLEMENTATION

#include <cgltf/cgltf.h>

#include <imgui.h>

int main() {
  auto display = cr::display(1920 * 2, 1080 * 2);

  auto configuration = cr::scene_configuration(
      glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 1024, 1024, 80.2f, 5);
  auto configuration_mutex = std::mutex();
  auto get_configuration = [&configuration,
      &configuration_mutex]() -> cr::scene_configuration {
    std::lock_guard lk(configuration_mutex);
    return configuration;
  };

  auto scenes = std::vector<cr::scene<cr::triangular_scene>>();
  auto frame = cr::atomic_image(configuration.width(), configuration.height());

  auto rendering = std::atomic<bool>(true);

  auto reset_sample_count = std::atomic<bool>(false);
  auto sample_count = uint64_t(0);
//  auto triangular_scene =
//      cr::triangular_scene("./assets/SM_Deccer_Cubes_Textured.glb");
//  auto scene = cr::scene<cr::triangular_scene>(&triangular_scene);

  auto cpu_renderer = cr::cpu_renderer();

  auto render_thread =
      std::thread([&rendering, &scenes, &get_configuration, &frame,
                   &sample_count, &reset_sample_count, &cpu_renderer] {
        const auto cpu_thread_count = std::thread::hardware_concurrency();
        fmt::print("count {}", cpu_thread_count);
        while (rendering) {
          auto configuration = get_configuration();

          auto tasks = configuration.get_tasks(cpu_thread_count);

          if (reset_sample_count) {
            sample_count = 0;
            reset_sample_count = false;
          }

          {
            auto data = cr::render_data {
                .samples = sample_count,
                .buffer = &frame,
                .intersect = [&scenes](const cr::ray &ray) {
                  auto result = std::optional<cr::intersection>();

                  for (auto &scene : scenes) {
                    auto intersection = scene.intersect(ray);
                    if (intersection) {
                      if (!result || result->distance > intersection->distance) {
                        result = intersection;
                      }
                    }
                  }

                  return result;
                }, // im not sure if this is the best way to do this
                .config = configuration,
            };

            cpu_renderer.render(data, tasks);

            cpu_renderer.wait();
          }


          sample_count += 1;
        }
      });

  auto mouse_pos_initialized = false;
  auto previous_mouse_pos = glm::vec2();

  while (!display.should_close()) {
    auto lines = std::vector<std::string>();

    auto origin = glm::vec3();

    if (display.key_down(cr::keyboard::key_code::key_s))
      origin.z += -1.0f;
    if (display.key_down(cr::keyboard::key_code::key_w))
      origin.z += 1.0f;
    if (display.key_down(cr::keyboard::key_code::key_a))
      origin.x += -1.0f;
    if (display.key_down(cr::keyboard::key_code::key_d))
      origin.x += 1.0f;

    auto rotation = glm::vec3();
    if (display.key_down(cr::keyboard::key_code::key_i))
      rotation.y += -1.0f;
    if (display.key_down(cr::keyboard::key_code::key_k))
      rotation.y += 1.0f;
    if (display.key_down(cr::keyboard::key_code::key_j))
      rotation.x += -1.0f;
    if (display.key_down(cr::keyboard::key_code::key_l))
      rotation.x += 1.0f;

    const auto mouse_pos = glm::vec2(display.mouse_position());
    if (!mouse_pos_initialized) {
      mouse_pos_initialized = true;
      previous_mouse_pos = mouse_pos;
    } else {
      const auto delta = previous_mouse_pos - mouse_pos;
    }

    if (origin != glm::vec3(0.0f) || rotation != glm::vec3(0.0f)) {
      std::lock_guard lk(configuration_mutex);

      reset_sample_count = true;

      const auto matrix = configuration.matrix();
      const auto translated_point =
          configuration.rotation_matrix() * glm::vec4(origin, 1.0f);

      configuration = cr::scene_configuration(
          glm::vec3(translated_point) + configuration.origin(),
          rotation + configuration.rotation(), configuration.width(),
          configuration.height(), configuration.fov(), configuration.bounces());
    }

    {
      display.render({
          .frame = &frame,
          .lines = &lines,
      });
    }
  }
  // Todo: handle exiting if we're sampling properly (signal the threads to die)

  rendering = false;
  render_thread.join();
}
