//
// Created by Caio W on 12/2/22.
//

#ifndef CREBON_DISPLAY_HPP
#define CREBON_DISPLAY_HPP

#include <mutex>
#include <variant>

#include <ui/input_devices.hpp>

#include <util/image.hpp>
#include <util/logger.hpp>

#include <GLFW/glfw3.h>
#include <fmt/format.h>

#include <ui/components/console.hpp>
#include <ui/components/preview.hpp>
#include <ui/components/settings.hpp>
#include <ui/components/stats.hpp>

#include <ui/components/component.hpp>


namespace cr {
class display {
public:
  display(size_t width, size_t height, cr::logger *logger);

  ~display();

  [[nodiscard]] bool should_close() const noexcept;

  struct render_data {
    cr::atomic_image *frame;
    std::vector<std::pair<cr::logger::level, std::string>> *lines;
    cr::component::stats::DisplayContents stats;
    cr::component::post_processing::Options post_processing;
  };
  struct user_input : public component::settings::Options {};
  [[nodiscard]] user_input render(render_data data);

  [[nodiscard]] bool key_down(cr::keyboard::key_code key) const noexcept;

  [[nodiscard]] glm::ivec2 mouse_position() const noexcept;

private:
  cr::keyboard _keyboard;
  cr::mouse _mouse;

  size_t _width;
  size_t _height;
  GLFWwindow *_window;

  cr::logger *_logger;

  struct {
    component::interface<component::preview> preview{};
    component::interface<component::console> console{};
    component::interface<component::settings> settings{};
    component::interface<component::stats> stats{};
  } _components{};

  static void _glfw_cursor_position_callback(GLFWwindow *window, double x,
                                             double y);

  static void _glfw_mouse_button_callback(GLFWwindow *window, int button,
                                          int action, int mods);

  static void _glfw_key_callback(GLFWwindow *window, int key, int scancode,
                                 int action, int mods);
};
} // namespace cr

#endif // CREBON_DISPLAY_HPP
