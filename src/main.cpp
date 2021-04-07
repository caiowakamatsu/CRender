#include <ui/display.h>
#include <util/model_loader.h>

int main()
{
    auto main_display = cr::display();

    {
        auto thread_pool = std::make_unique<cr::thread_pool>(8);

        const auto camera = cr::camera();

        const auto scene = std::make_unique<cr::new_scene>();

        auto renderer = cr::new_renderer(
          camera,
          1024,
          1024,
          &thread_pool,
          &scene);
    }


    /*
     * Creating the renderer
     */
    auto render_config         = cr::renderer::config();
    render_config.thread_count = 16;
    render_config.resolution.x = 512;
    render_config.resolution.y = 512;
    render_config.render_mode  = cr::renderer::config::LINE;

    auto renderer = std::make_unique<cr::renderer>(render_config);

    /*
     * Creating the scene and loading a model in
     */
    auto scene = std::make_unique<cr::scene>();
    //    const auto model = cr::model::load_model("./assets/tea.obj");
    //
    //    scene->add_triangles(model.vertices, model.vertex_indices);
    //    scene->add_materials(model.materials, model.material_indices);

    // Attach scene to renderer
    renderer->attach_scene(std::move(scene));

    // Start rendering
    renderer->start();

    main_display.register_prerender_callback([&renderer](cr::display &display) {
        auto response = cr::display::pre_render_response();

        response.to_display = std::move(renderer->current_progress());
        response.renderer   = renderer.get();

        return response;
    });

    main_display.register_updated_renderer_callback(
      [&renderer](std::unique_ptr<cr::renderer> updated_renderer) {
          renderer->stop();
          renderer = std::move(updated_renderer);
          renderer->start();
      });

    main_display.register_postrender_callback(
      [&renderer](cr::display &display, cr::display::post_render_result result) {
          if (result.render_display_config.has_value() || result.updated_materials.has_value())
              renderer->stop();

          if (result.render_display_config.has_value())
          {
              auto &new_user_config = result.render_display_config.value();
              renderer              = renderer->from_config(new_user_config);
          }

          if (result.updated_materials.has_value())
              renderer->scene()->update_materials(result.updated_materials.value());

          if (result.render_display_config.has_value() || result.updated_materials.has_value())
              renderer->start();
      });

    main_display.start();
}
