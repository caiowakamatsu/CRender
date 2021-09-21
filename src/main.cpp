#include <ui/display.h>

int main()
{
    const auto thread_count = std::thread::hardware_concurrency();
    auto thread_pool = std::make_unique<cr::thread_pool>(thread_count == 0 ? 1 : thread_count / 4);

    auto scene = std::make_unique<cr::scene>();

    auto renderer = std::make_unique<cr::renderer>(1024, 1024, 5, &thread_pool, &scene);
    auto main_display = cr::display();

    auto draft_renderer = std::make_unique<cr::draft_renderer>(1024, 1024, &scene);

    auto gpu_renderer = std::make_unique<cr::gpu_renderer>(scene.get(), glm::ivec2(512, 512));

    main_display.start(scene, renderer, thread_pool, draft_renderer, gpu_renderer);
}
