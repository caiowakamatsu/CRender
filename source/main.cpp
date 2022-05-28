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
  auto render_target_options = cr::component::render_target::Options();

  auto display = cr::display(1920, 1080);

  auto configuration = cr::scene_configuration(
      glm::vec3(0, 0, -20), glm::vec3(0, 0, 0), 1024, 1024, 80.2f, render_target_options.ray_depth);
  auto settings = cr::display::user_input();

  auto scenes = std::vector<cr::scene<cr::triangular_scene>>();
  auto frame = cr::atomic_image(configuration.width(), configuration.height());
  auto frame_mutex = std::mutex();

  auto rendering = std::atomic<bool>(true);

  auto reset_sample_count = std::atomic<bool>(false);
  auto sample_count = uint64_t(0);
  auto triangular_scene =
      cr::triangular_scene("./assets/models/SM_Deccer_Cubes_Textured.glb");
  scenes.emplace_back(&triangular_scene);

  auto intersect_scenes =
      [&](const cr::ray &ray) -> std::optional<cr::intersection> {
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
  };

  auto cpu_renderer =
      cr::cpu_renderer(int(std::thread::hardware_concurrency()), {}, render_target_options.samples_per_pixel);

  auto tasks = configuration.get_tasks(std::thread::hardware_concurrency());
  cpu_renderer.start(
      cr::render_data{
          .buffer = &frame,
          .intersect = intersect_scenes,
          .config = configuration,
      },
      tasks);

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

    const auto input = [&]() {
      //      std::lock_guard frame_lk(frame_mutex);
      return display.render({
          .frame = &frame,
          .lines = &lines,
      });
    }();

    auto update_anything = false;
    update_anything |= origin != glm::vec3() || rotation != glm::vec3();
    update_anything |= input.skybox.has_value();
    update_anything |= input.render_target.has_value();

    if (update_anything) {
      cpu_renderer.stop();

      const auto matrix = configuration.matrix();
      const auto translated_point =
          configuration.rotation_matrix() * glm::vec4(origin, 1.0f);

      const auto new_width = input.render_target.has_value()
                                 ? input.render_target->resolution.x
                                 : configuration.width();
      const auto new_height = input.render_target.has_value()
                                  ? input.render_target->resolution.y
                                  : configuration.height();

      if (new_width != configuration.width() ||
          new_height != configuration.height()) {
        frame = cr::atomic_image(new_width, new_height);
      }

      configuration = cr::scene_configuration(
          glm::vec3(translated_point) + configuration.origin(),
          rotation + configuration.rotation(), new_width, new_height,
          configuration.fov(), configuration.bounces());

      if (input.skybox.has_value()) {
        cpu_renderer.sky.use_settings(input.skybox.value());
      }

      tasks = configuration.get_tasks(std::thread::hardware_concurrency());
      cpu_renderer.start(cr::render_data {
              .buffer = &frame,
              .intersect = intersect_scenes,
              .config = configuration,
          },
          tasks);
    }
  }
  // Todo: handle exiting if we're sampling properly (signal the threads to die)

  cpu_renderer.stop();

  rendering = false;
  //  render_thread.join();
}
