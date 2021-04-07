#include "video_writer.h"

cr::video_writer::video_writer(const std::string &file, uint64_t width, uint64_t height)
    : _width(width), _height(height), _current_frame(0)
{
    avformat_alloc_output_context2(&_output_context, nullptr, nullptr, file.c_str());

    _sws_context = sws_getContext(
      _width,
      _height,
      AV_PIX_FMT_RGB24,
      _width,
      _height,
      AV_PIX_FMT_YUV420P,
      SWS_FAST_BILINEAR,
      nullptr,
      nullptr,
      nullptr);

    // Setup the codec
    auto          codec  = avcodec_find_encoder(AV_CODEC_ID_VP9);
    _stream = avformat_new_stream(_output_context, codec);

    _output_format = av_guess_format("mp4", nullptr, nullptr);

    _codec_context            = _stream->codec;    // I KNOW I KNOW DEPRECATED LET ME BE
    _codec_context->width     = width;
    _codec_context->height    = height;
    _codec_context->pix_fmt   = AV_PIX_FMT_YUV420P;

    auto time_base = AVRational();
    time_base.num = 1;
    time_base.den = 25;
    _codec_context->time_base = time_base;

    if (_output_context->oformat->flags & AVFMT_GLOBALHEADER)
        _codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    avcodec_open2(_codec_context, codec, nullptr);

    _stream->time_base = time_base;
    av_dump_format(_output_context, 0, file.c_str(), 1);
    avio_open(&_output_context->pb, file.c_str(), AVIO_FLAG_WRITE);
    auto ret = avformat_write_header(_output_context, nullptr);

    _rgb_frame         = av_frame_alloc();
    _rgb_frame->format = AV_PIX_FMT_RGB24;
    _rgb_frame->width  = _width;
    _rgb_frame->height = _height;
    ret                = av_frame_get_buffer(_rgb_frame, 1);

    _yuv_frame         = av_frame_alloc();
    _yuv_frame->format = AV_PIX_FMT_YUV420P;
    _yuv_frame->width  = _width;
    _yuv_frame->height = _height;
    ret                = av_frame_get_buffer(_yuv_frame, 1);
}

void cr::video_writer::submit_frame(const cr::image &frame)
{
    for (auto y = 0; y < _height; y++)
    {
        for (auto x = 0; x < _width; x++)
        {
            const auto colour_at = frame.get(x, y);

            _rgb_frame->data[0][y * _rgb_frame->linesize[0] + 3 * x + 0] = colour_at.b * 255.f;
            _rgb_frame->data[0][y * _rgb_frame->linesize[0] + 3 * x + 1] = colour_at.g * 255.f;
            _rgb_frame->data[0][y * _rgb_frame->linesize[0] + 3 * x + 2] = colour_at.r * 255.f;
        }
    }

    // Not scaling, just translating the RGB to YUV and storing it
    sws_scale(
      _sws_context,
      _rgb_frame->data,
      _rgb_frame->linesize,
      0,
      _height,
      _yuv_frame->data,
      _yuv_frame->linesize);

    av_init_packet(&_packet);
    _packet.data = nullptr;
    _packet.size = 0;

    _yuv_frame->pts = _current_frame++;

    auto got_output = int();

    [[maybe_unused]] auto ret =
      avcodec_encode_video2(_codec_context, &_packet, _yuv_frame, &got_output);

    if (got_output)
    {
        auto time_base = AVRational();
        time_base.num = 1;
        time_base.den = 25;
        av_packet_rescale_ts(&_packet, time_base, _stream->time_base);

        _packet.stream_index = _stream->index;

        av_interleaved_write_frame(_output_context, &_packet);
        av_packet_unref(&_packet);
    }
}

void cr::video_writer::write()
{
    av_write_trailer(_output_context);

    if (!(_output_format->flags & AVFMT_NOFILE)) avio_closep(&_output_context->pb);
    avcodec_close(_stream->codec);

    sws_freeContext(_sws_context);
    av_frame_free(&_rgb_frame);
    av_frame_free(&_yuv_frame);
    avformat_free_context(_output_context);
}
