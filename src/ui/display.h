#include <functional>
#include <optional>

#include <GLFW/glfw3.h>
#include <util/exception.h>

#include <fmt/core.h>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>
#include <imgui/imnodes.h>

#include <glad/glad.h>

#include <ui/themes.h>
#include <objects/image.h>

namespace cr
{
    class display
    {
    public:
        struct pre_render_response
        {
            cr::image *to_display = nullptr;
        };

        struct post_render_result
        {
        };

        display();

        void start();

        void stop();

        void register_close_callback(const std::function<void(cr::display &)> &callback);

        void register_prerender_callback(
          const std::function<pre_render_response(cr::display &)> &callback);

        void register_postrender_callback(
          const std::function<post_render_result(cr::display &)> &callback);


    private:
        std::optional<std::function<void(cr::display &)>>                _close_callback;
        std::optional<std::function<pre_render_response(cr::display &)>> _prerender_callback;
        std::optional<std::function<post_render_result(cr::display &)>>  _postrender_callback;

        GLFWwindow* _glfw_window;

        uint _main_texture_handle{};
    };
}    // namespace cr
