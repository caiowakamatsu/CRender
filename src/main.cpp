#include <ui/display.h>
#include <codecs/video_reader.h>
#include <codecs/video_writer.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

int main()
{
    auto video_reader = cr::video_reader();
    auto video_writer = cr::video_writer("test_poop.mp4", video_reader.width(), video_reader.height());

    auto main_display = cr::display();

    constexpr auto max_frames = 600;
    int current_frame = 0;

    main_display.register_prerender_callback([&video_reader, &video_writer, &current_frame](cr::display &display) {
        auto response = cr::display::pre_render_response();


        const auto frame = video_reader.next_frame();
        response.to_display = frame;
        video_writer.submit_frame(*frame);
        if (current_frame++ >= max_frames)
            display.stop();

        return response;
    });

    main_display.start();

    video_writer.write();
}
