#pragma once

#include <vector>

#include <ffmpeg/ffmpeg.h>
#include <util/exception.h>
#include <algorithm>

#include <objects/image.h>
#include <util/colour.h>

namespace cr
{
    class video_reader
    {
    public:
        video_reader();

        [[nodiscard]] cr::image *next_frame() noexcept;

        [[nodiscard]] cr::image *get_frame(uint64_t index) noexcept;

        [[nodiscard]] uint64_t width() const noexcept;

        [[nodiscard]] uint64_t height() const noexcept;

    private:
        AVFormatContext* _video_format;

        std::vector<AVStream*> _streams;
        std::vector<cr::image> _frames;

        uint64_t _video_width;
        uint64_t _video_height;
        int64_t _bit_rate;
        double _video_length;

        uint64_t _current_frame = 1;
    };
}
