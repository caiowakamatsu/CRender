#include <ui/display.h>

int main()
{
    constexpr auto START_RES = glm::ivec2(1024, 1024);

    const auto thread_count = std::thread::hardware_concurrency();
    auto thread_pool = std::make_unique<cr::thread_pool>(thread_count == 0 ? 1 : thread_count / 4);

    auto scene = std::make_unique<cr::scene>();

    auto renderer =
      std::make_unique<cr::renderer>(START_RES.x, START_RES.y, 5, &thread_pool, &scene);
    auto main_display = cr::display();

    auto draft_renderer = std::make_unique<cr::draft_renderer>(START_RES.x, START_RES.y, &scene);

    renderer->update(
      [&scene, &renderer]
      {
          const auto mis_model = cr::asset_loader::load_model(
            "./assets/models\\mis-texture\\mis.obj",
            "./assets/models\\mis-texture");
          scene->add_model(mis_model);
          renderer->set_resolution(1920, 1080);
      });

    main_display.start(scene, renderer, thread_pool, draft_renderer);
}
