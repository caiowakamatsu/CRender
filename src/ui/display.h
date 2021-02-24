#include <functional>



class Display
{
public:
    struct PreRenderResponse
    {
        const unsigned char *frame_to_display = nullptr;
    };

    struct PostRenderResult
    {
    };
    Display();

    void start();

    void stop();

    void register_prerender_callback(const std::function<PreRenderResponse(const Display &)>& callback);

    void register_postrender_callback(const std::function<PostRenderResult(const Display &)>& callback);

private:

    std::function<PreRenderResponse(const Display &)> _prerender_callback;
    std::function<PostRenderResult(const Display &)> _postrender_callback;

    void _render_frame();
};
