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
#include <render/draft/draft_renderer.h>
#include <util/model_loader.h>

namespace cr
{
    class display
    {
    public:
        display();

        void start(
          std::unique_ptr<cr::scene> *      scene,
          std::unique_ptr<cr::renderer> *   renderer,
          std::unique_ptr<cr::draft_renderer> *   draft_renderer,
          std::unique_ptr<cr::thread_pool> *thread_pool);

        void stop();

        ~display();

    private:

        void _reload_compute();

        GLFWwindow *_glfw_window;

        GLuint _scene_texture_handle = -1;
        GLuint _compute_shader_id = -1;
        GLuint _compute_shader_program = -1;
        GLuint _target_texture = -1;

        std::optional<uint64_t> _current_entity;
    };
}    // namespace cr
