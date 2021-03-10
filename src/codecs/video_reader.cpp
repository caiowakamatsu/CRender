#include "video_reader.h"

cr::video_reader::video_reader()
{
    _video_format = avformat_alloc_context();
    avformat_open_input(&_video_format, "assets/test_1.mp4", nullptr, nullptr);

    _video_length = static_cast<double>(_video_format->duration) / 1000000.0;
    _bit_rate     = _video_format->bit_rate;

    avformat_find_stream_info(_video_format, nullptr);
    _streams.reserve(_video_format->nb_streams);

    AVCodec *          codec            = nullptr;
    AVCodecParameters *codec_parameters = nullptr;

    auto pixels      = std::vector<uint32_t>();
    auto codec_index = -1;
    for (auto i = 0; i < _video_format->nb_streams; i++)
    {
        auto local_codec_parameters = _video_format->streams[i]->codecpar;
        auto local_codec            = avcodec_find_decoder(local_codec_parameters->codec_id);

        if (local_codec_parameters->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            codec_index = i;

            _video_width = local_codec_parameters->width;
            _video_height = local_codec_parameters->height;

            codec            = local_codec;
            codec_parameters = local_codec_parameters;
            pixels.resize(local_codec_parameters->width * local_codec_parameters->height);
        }
    }

    if (codec_index == -1) cr::exit("Could not load video from codec");

    auto codec_context = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_context, codec_parameters);

    avcodec_open2(codec_context, codec, nullptr);

    auto frame  = av_frame_alloc();
    auto packet = av_packet_alloc();

    int response = 0;
    fmt::print("Test");

    while (av_read_frame(_video_format, packet) >= 0)
    {
        if (packet->stream_index == codec_index)
        {
            response = avcodec_send_packet(codec_context, packet);
            while (response >= 0)
            {
                response = avcodec_receive_frame(codec_context, frame);

                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) break;

                for (auto y = 0; y < frame->height; y++)
                {
                    for (auto x = 0; x < frame->width; x++)
                    {
                        const auto Y = frame->data[0][frame->linesize[0] * y + x];

                        const auto Cb = frame->data[1][frame->linesize[1] * (y / 2) + x / 2];
                        const auto Cr = frame->data[2][frame->linesize[2] * (y / 2) + x / 2];

                        const auto colour = cr::colour::RGB_from_YCbCr(Y, Cb, Cr);

                        // This is an = to reset the previous pixels value
                        pixels[(x + y * frame->width)] = static_cast<uint8_t>(colour.r * 255.f);
                        pixels[(x + y * frame->width)] |= static_cast<uint8_t>(colour.g * 255.f)
                          << 8;
                        pixels[(x + y * frame->width)] |= static_cast<uint8_t>(colour.b * 255.f)
                          << 16;
                        pixels[(x + y * frame->width)] |= static_cast<uint8_t>(~0) << 24;
                    }
                }

                if (frame->width != 0 && frame->height != 0)
                    _frames.emplace_back(pixels, frame->width, frame->height);
            }
        }
    }
}

cr::image *cr::video_reader::next_frame() noexcept
{
    return _frames.data() + std::max(1UL, _current_frame++) % _frames.size();
}

cr::image *cr::video_reader::get_frame(uint64_t index) noexcept
{
    return &_frames[index];
}

uint64_t cr::video_reader::width() const noexcept
{
    return _video_width;
}

uint64_t cr::video_reader::height() const noexcept
{
    return _video_height;
}
