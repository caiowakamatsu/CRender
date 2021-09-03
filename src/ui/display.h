#pragma once

#include <functional>
#include <optional>
#include <fstream>
#include <streambuf>
#include <sstream>

#include <GLFW/glfw3.h>
#include <util/exception.h>

#include <fmt/core.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>
#include <imgui/imnodes.h>

#include <glad/glad.h>

#include <ui/themes.h>
#include <ui/ui.h>
#include <objects/image.h>
#include <render/renderer.h>
#include <render/timer.h>
#include <render/draft/draft_renderer.h>
#include <util/asset_loader.h>
#include <util/logger.h>

namespace cr
{
    class display
    {
    public:

        enum class action
        {
            RELEASE,
            PRESS,
            REPEAT,
            UNKNOWN
        };

        enum class mouse
        {
            LEFT,
            RIGHT,
            MIDDLE,
            OTHER
        };

        display();

        void start(
          std::unique_ptr<cr::scene> &         scene,
          std::unique_ptr<cr::renderer> &      renderer,
          std::unique_ptr<cr::thread_pool> &   thread_pool,
          std::unique_ptr<cr::draft_renderer> &draft_renderer,
          std::unique_ptr<cr::post_processor> &post_processor);

        void stop();

        ~display();

    private:

        void _poll_events();

        void _update_camera(cr::camera *camera);

        GLFWwindow *_glfw_window;

        GLuint _scene_texture_handle   = -1;
        GLuint _compute_shader_id      = -1;
        GLuint _compute_shader_program = -1;
        GLuint _target_texture         = -1;

        cr::timer _timer;

        bool _in_draft_mode = false;

        glm::vec2 _mouse_pos {};
        glm::vec2 _mouse_pos_prev {};
        glm::vec2 _mouse_change_prev {};

        std::optional<uint64_t> _current_entity;

        std::array<key_state, static_cast<size_t>(key_code::MAX_KEY)> _key_states {};

        std::unordered_map<std::string, ImFont*> _fonts;

    };
}    // namespace cr
