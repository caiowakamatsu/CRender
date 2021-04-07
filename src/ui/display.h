#include <functional>
#include <optional>

#include <GLFW/glfw3.h>
#include <util/exception.h>

#include <fmt/core.h>

#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>
#include <imgui/imnodes.h>

#include <glad/glad.h>

#include <ui/themes.h>
#include <objects/image.h>
#include <render/renderer.h>
#include <util/model_loader.h>

namespace cr
{
    class display
    {
    public:
        struct pre_render_response
        {
            cr::renderer *           renderer = nullptr;
            std::optional<cr::image> to_display;
        };

        struct post_render_result
        {
            std::optional<cr::renderer::config>      render_display_config;
            std::optional<std::vector<cr::material>> updated_materials;
        };

        explicit display();

        void start();

        void stop();

        void register_updated_renderer_callback(
          const std::function<void(std::unique_ptr<renderer>)> &callback);

        void register_close_callback(const std::function<void(cr::display &)> &callback);

        void register_prerender_callback(
          const std::function<pre_render_response(cr::display &)> &callback);

        void register_postrender_callback(
          const std::function<void(cr::display &, post_render_result)> &callback);

    private:
        std::optional<std::function<void(std::unique_ptr<renderer>)>>    _updated_renderer_callback;
        std::optional<std::function<void(cr::display &)>>                _close_callback;
        std::optional<std::function<pre_render_response(cr::display &)>> _prerender_callback;
        std::optional<std::function<void(cr::display &, post_render_result)>> _postrender_callback;

        GLFWwindow *_glfw_window;

        uint64_t _main_texture_handle {};

        // IMGUI STUFF
        struct ImGuiSetup
        {
            ImGuiWindowFlags window_flags;
            ImGuiViewport *  viewport;
        };
        [[nodiscard]] ImGuiSetup IMGUI_init(const ImGuiDockNodeFlags &dockspace_flags);

        static void IMGUI_setup_dock(
          const ImGuiDockNodeFlags &dockspace_flags,
          const ImGuiWindowFlags &  flags,
          const ImGuiViewport *     viewport);

        static void IMGUI_scene_preview(
          const cr::display::pre_render_response &pre_render_response,
          uint64_t                                texture_handle);

        struct SceneEditorResponse
        {
        };
        [[nodiscard]] SceneEditorResponse IMGUI_scene_editor(cr::renderer *renderer);

        [[nodiscard]] static std::optional<std::filesystem::path> IMGUI_model_loader();

        [[nodiscard]] static int
          IMGUI_material_list(const std::vector<cr::material> &material_list);

        enum class _SelectedType
        {
            POINT_LIGHT,
            DIRECTIONAL_LIGHT,
            MATERIAL,
            NONE
        };
        _SelectedType _current_selected_type = _SelectedType::NONE;
        union
        {
            cr::point_light *      point_light = nullptr;
            cr::directional_light *directional_light;
            cr::material *         material;
        } _selected_object;
    };
}    // namespace cr
