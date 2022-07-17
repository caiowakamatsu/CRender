#include <cmath>

#include <render/cpu_renderer.h>

#include <scene/scene.hpp>
#include <scene/sphere_scene.hpp>
#include <scene/triangular_scene.hpp>

#include <ui/display.hpp>

#include <CRender.hpp>

#include <fmt/core.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include <stb/stb_image_write.h>

#define TINYOBJLOADER_IMPLEMENTATION

#include <tinyobjloader/tinyobjloader.h>

#define TINYGLTF_IMPLEMENTATION

#include <tinygltf/tinygltf.h>

#define CGLTF_IMPLEMENTATION

#include <cgltf/cgltf.h>

#include <util/logger.hpp>
#include <util/denoise.hpp>

int main()
{

// Below this
  auto logger = cr::logger();

  auto render_target_options = cr::component::render_target::Options();

  auto post_processing_options = cr::component::post_processing::Options();

  auto display = cr::display(1920, 1080, &logger);

  auto configuration =
      cr::scene_configuration(glm::vec3(0, 0, -20), glm::vec3(0, 0, 0), 1024,
                              1024, 80.2f, render_target_options.ray_depth);
  auto settings = cr::display::user_input();

  auto triangular_scenes = std::vector<std::unique_ptr<cr::triangular_scene>>();
  auto scenes = std::vector<cr::scene<cr::triangular_scene>>();
  auto frame = cr::atomic_image(configuration.width(), configuration.height());
  auto normal_frame = cr::atomic_image(configuration.width(), configuration.height());
  auto albedo_frame = cr::atomic_image(configuration.width(), configuration.height());

  auto sample_count = uint64_t(0);

  auto intersect_scenes =
      [&](const cr::ray &ray) -> std::optional<cr::intersection>
  {
    auto result = std::optional<cr::intersection>();

    for (auto &scene : scenes)
    {
      auto intersection = scene.intersect(ray);
      if (intersection)
      {
        if (!result || result->distance > intersection->distance)
        {
          result = intersection;
        }
      }
    }

    return result;
  };


// Above this Gon be in the class


  auto cpu_renderer =
      cr::cpu_renderer(int(std::thread::hardware_concurrency()), {},
                       render_target_options.samples_per_pixel);

// These too
  auto tasks = configuration.get_tasks(std::thread::hardware_concurrency());
  cpu_renderer.start(
      cr::render_data{
          .buffer = &frame,
          .normal_buffer = &normal_frame,
          .albedo_buffer = &albedo_frame,
          .intersect = intersect_scenes,
          .config = configuration,
      },
      tasks);

  auto mouse_pos_initialized = false;
  auto previous_mouse_pos    = glm::vec2();
// Uptill this


  while (!display.should_close())
  {
    auto lines = std::vector<std::string>();

    auto origin = glm::vec3();
    auto rotation = glm::vec3();

    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
      if (display.key_down(cr::keyboard::key_code::key_s))
        origin.z += -1.0f;
      if (display.key_down(cr::keyboard::key_code::key_w))
        origin.z += 1.0f;
      if (display.key_down(cr::keyboard::key_code::key_a))
        origin.x += -1.0f;
      if (display.key_down(cr::keyboard::key_code::key_d))
        origin.x += 1.0f;

      if (display.key_down(cr::keyboard::key_code::key_i))
        rotation.y += -1.0f;
      if (display.key_down(cr::keyboard::key_code::key_k))
        rotation.y += 1.0f;
      if (display.key_down(cr::keyboard::key_code::key_j))
        rotation.x += -1.0f;
      if (display.key_down(cr::keyboard::key_code::key_l))
        rotation.x += 1.0f;
    }

// jere
    const auto mouse_pos = glm::vec2(display.mouse_position());
    if (!mouse_pos_initialized)
    {
      mouse_pos_initialized = true;
      previous_mouse_pos = mouse_pos;
    }
    else
    {
      const auto delta = previous_mouse_pos - mouse_pos;
    }
// down to here

// why tf
// but I hafta to include it too ig
    const auto total_sample_count = cpu_renderer.total_samples();
    const auto render_time = cpu_renderer.total_time();

    const auto input = [&]()
    {
      auto logs = logger.logs();

      return display.render(
          {.frame = &frame,
           .lines = &logs,
           .stats = {.samples_per_second =
                         static_cast<int>(total_sample_count / render_time),
                     .total_samples = static_cast<int>(total_sample_count),
                     .rays_per_second = static_cast<int>(
                         cpu_renderer.total_rays() / render_time),
                     .total_instances = static_cast<int>(scenes.size()),
                     .total_render_time = render_time},
           .post_processing = post_processing_options});
    }();


// THIS IS IN THER 4 SURE
    auto update_anything = false;
    update_anything |= origin != glm::vec3() || rotation != glm::vec3();
    update_anything |= input.skybox.has_value();
    update_anything |= input.render_target.has_value();
    update_anything |= input.asset_loader.has_value();
    update_anything |= input.image_export.has_value();
    update_anything |= input.post_processing.has_value();

    if (update_anything)
    {
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
          new_height != configuration.height())
      {
        frame = cr::atomic_image(new_width, new_height);
        normal_frame = cr::atomic_image(new_width, new_height);
        albedo_frame = cr::atomic_image(new_width, new_height);
      }

      // Why is this here
      configuration = cr::scene_configuration(
          glm::vec3(translated_point) + configuration.origin(),
          rotation + configuration.rotation(), new_width, new_height,
        vec_of_functions  configuration.fov(), configuration.bounces());

      if (input.skybox.has_value())
      {
        cpu_renderer.skybox.use_settings(input.skybox.value());
      }

      if (input.asset_loader.has_value())
      {
        auto scene = std::make_unique<cr::triangular_scene>(
            input.asset_loader->load, &logger);
        triangular_scenes.push_back(std::move(scene));
        scenes.emplace_back(triangular_scenes.back().get());
      }

      if (input.post_processing.has_value())
      {
        post_processing_options = input.post_processing.value();
      }

      if (input.image_export.has_value())
      {
        // Export image
        std::filesystem::create_directories("./out/" + input.image_export->scene_name);

        // 0 = jpg
        // 1 = exr
        if (input.image_export->image_type == 0)
        {
          auto data = frame.view().data;
          const auto width = frame.width();
          const auto height = frame.height();
          auto char_data = std::vector<uint8_t>(width * height * 4);

          if (post_processing_options.denoise)
          {
            auto framebuffer_view = frame.view();
            auto normal_view = normal_frame.view();
            auto albedo_view = albedo_frame.view();
            data = cr::denoise(&framebuffer_view, &normal_view, &albedo_view);
          }

          for (auto y = 0; y < height; ++y)
          {
            for (auto x = 0; x < width; ++x)
            {
              const auto base = (x + y * width) * 3;
              const auto write_base = (x + (height - y - 1) * width) * 4;

              const auto pixel =
                  glm::vec3(data[base + 0], data[base + 1], data[base + 2]);

              auto post_processed = pixel * post_processing_options.exposure;

              if (post_processing_options.gamma_correct)
              {
                post_processed = glm::pow(post_processed, glm::vec3(1.0f / 2.2f));
              }

              // where the image gets written to a jpg image
              char_data[write_base + 0] = static_cast<uint8_t>(
                  glm::clamp(post_processed.x * 255.0f, 0.0f, 255.0f));
              char_data[write_base + 1] = static_cast<uint8_t>(
                  glm::clamp(post_processed.y * 255.0f, 0.0f, 255.0f));
              char_data[write_base + 2] = static_cast<uint8_t>(
                  glm::clamp(post_processed.z * 255.0f, 0.0f, 255.0f));
              char_data[write_base + 3] = 255;
            }
          }

          auto path =
              fmt::format("./out/{}/render.jpg", input.image_export->scene_name);
          stbi_write_jpg(path.c_str(), width, height, 4, char_data.data(), 100);
        }

        // Exr output
        else if(input.image_export->image_type == 1)
        {
          std::cout << "uwu" << '\n';
        }


      }

      tasks = configuration.get_tasks(std::thread::hardware_concurrency());
      cpu_renderer.start(
          cr::render_data{
              .buffer = &frame,
              .normal_buffer = &normal_frame,
              .albedo_buffer = &albedo_frame,
              .intersect = intersect_scenes,
              .config = configuration,
          },
          tasks);
    }
  }
  // Todo: handle exiting if we're sampling properly (signal the threads to die)

  cpu_renderer.stop();
}


/* 
*/