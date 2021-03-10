#pragma once

#include <string>

#include <ffmpeg/ffmpeg.h>

#include <objects/image.h>

namespace cr
{
    class video_writer
    {
    public:
        video_writer(const std::string &file, uint64_t width, uint64_t height);

        void submit_frame(const cr::image &frame);

        void write();

    private:
        SwsContext *_sws_context;

        uint64_t _current_frame;
        uint64_t _width;
        uint64_t _height;

        AVFormatContext *_output_context = nullptr;
        AVOutputFormat * _output_format  = nullptr;
        AVCodecContext * _codec_context  = nullptr;
        AVStream *       _stream         = nullptr;
        AVPacket         _packet;

        AVFrame *_rgb_frame, *_yuv_frame;

        std::vector<cr::image> _frame_buffer;
    };
}    // namespace cr
