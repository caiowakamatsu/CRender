#include <ui/display.h>

int main()
{
    auto thread_pool = std::make_unique<cr::thread_pool>(8);

    auto scene = std::make_unique<cr::scene>();

    auto renderer = std::make_unique<cr::renderer>(1024, 1024, 5, &thread_pool, &scene);

    auto main_display = cr::display();

    main_display.start(&scene, &renderer, &thread_pool);
}
