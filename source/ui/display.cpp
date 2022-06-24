//
// Created by Caio W on 12/2/22.
//

#include "display.hpp"

#include <glad/glad.h>

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace {
struct init_ctx {
  ImGuiDockNodeFlags dock_flags;
  ImGuiWindowFlags window_flags;
  ImGuiViewport *viewport;
};
[[nodiscard]] inline init_ctx init() {
  auto dock_flags = ImGuiDockNodeFlags_PassthruCentralNode;
  auto window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  auto viewport = ImGui::GetMainViewport();

  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  window_flags |=
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  if (dock_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  auto ctx = init_ctx();
  ctx.dock_flags = dock_flags;
  ctx.window_flags = window_flags;
  ctx.viewport = viewport;
  return ctx;
}

inline void init_dock(const init_ctx &ctx, const std::string &top_left,
                      const std::string &bottom_left,
                      const std::string &right_panel,
                      const std::string &right_panel_bottom) {
  auto dockspace_id = ImGui::GetID("DockSpace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ctx.dock_flags);

  static auto first_time = true;
  if (first_time) {
    first_time = false;

    ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
    ImGui::DockBuilderAddNode(dockspace_id,
                              ctx.dock_flags | ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ctx.viewport->Size);

    auto dock_id_right = ImGui::DockBuilderSplitNode(
        dockspace_id, ImGuiDir_Right, 0.2f, nullptr, &dockspace_id);

    auto dock_id_bottom_right = ImGui::DockBuilderSplitNode(
        dock_id_right, ImGuiDir_Down, 0.22f, nullptr,
        &dock_id_right); // .22f is a better looking default, trust me...

    auto dock_id_down = ImGui::DockBuilderSplitNode(
        dockspace_id, ImGuiDir_Down, 0.2f, nullptr, &dockspace_id);

    ImGui::DockBuilderGetNode(dock_id_right)->LocalFlags |=
        ImGuiDockNodeFlags_NoTabBar;
    ImGui::DockBuilderGetNode(dock_id_down)->LocalFlags |=
        ImGuiDockNodeFlags_NoTabBar;
    ImGui::DockBuilderGetNode(dockspace_id)->LocalFlags |=
        ImGuiDockNodeFlags_NoTabBar;
    ImGui::DockBuilderGetNode(dock_id_bottom_right)->LocalFlags |=
        ImGuiDockNodeFlags_NoTabBar;

    ImGui::DockBuilderDockWindow(top_left.c_str(), dockspace_id);
    ImGui::DockBuilderDockWindow(right_panel.c_str(), dock_id_right);
    ImGui::DockBuilderDockWindow(bottom_left.c_str(), dock_id_down);
    ImGui::DockBuilderDockWindow(right_panel_bottom.c_str(),
                                 dock_id_bottom_right);

    ImGui::DockBuilderFinish(dockspace_id);
  }
}

inline void root_node(init_ctx ui_ctx) {
  ImGui::Begin("DockSpace", nullptr, ui_ctx.window_flags);
  ImGui::PopStyleVar(3);

  // Set up the dock
  ::init_dock(ui_ctx, "Preview", "Console", "Settings", "Stats");

  ImGui::End();
}
} // namespace

cr::display::display(size_t width, size_t height, cr::logger *logger)
    : _width(width), _height(height), _logger(logger) {
  if (!glfwInit()) {
    logger->error("Failed to initialize GLFW");
    exit(-1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);

  _window = glfwCreateWindow(_width, _height, "CRender", NULL, NULL);
  if (!_window) {
    logger->error("Failed to create GLFW window");
    exit(-1);
  }

  glfwMakeContextCurrent(_window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  glfwSetWindowUserPointer(_window, this);

  glfwSetCursorPosCallback(_window, _glfw_cursor_position_callback);
  glfwSetMouseButtonCallback(_window, _glfw_mouse_button_callback);
  glfwSetKeyCallback(_window, _glfw_key_callback);

  ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *)_window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

cr::display::~display() { glfwTerminate(); }

bool cr::display::should_close() const noexcept {
  return glfwWindowShouldClose(_window);
}

cr::display::user_input cr::display::render(render_data data) {
  glClearColor(.2f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  auto ui_ctx = ::init();
  ::root_node(ui_ctx);

  auto input = user_input();

  [[maybe_unused]] const auto preview = _components.preview.display(
      {.frame = data.frame, .post = data.post_processing});

  [[maybe_unused]] const auto console =
      _components.console.display({.lines = data.lines});

  [[maybe_unused]] const auto stats = _components.stats.display(data.stats);

  const auto settings = _components.settings.display({});
  input.render_target = settings.render_target;
  input.image_export = settings.image_export;
  input.asset_loader = settings.asset_loader;
  input.skybox = settings.skybox;
  input.post_processing = settings.post_processing;

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  glfwSwapBuffers(_window);
  glfwPollEvents();

  return input;
}

void cr::display::_glfw_cursor_position_callback(GLFWwindow *window, double x,
                                                 double y) {
  auto user_window =
      static_cast<cr::display *>(glfwGetWindowUserPointer(window));
  user_window->_mouse.set_position({x, y});
}

void cr::display::_glfw_mouse_button_callback(GLFWwindow *window, int button,
                                              int action, int mods) {
  auto user_window =
      static_cast<cr::display *>(glfwGetWindowUserPointer(window));

  auto mouse_button = cr::mouse::button::left;
  switch (button) {
  case GLFW_MOUSE_BUTTON_LEFT:
    mouse_button = cr::mouse::button::left;
    break;
  case GLFW_MOUSE_BUTTON_RIGHT:
    mouse_button = cr::mouse::button::right;
    break;
  case GLFW_MOUSE_BUTTON_MIDDLE:
    mouse_button = cr::mouse::button::middle;
    break;
  default:;
  }

  auto mouse_action = cr::mouse::button_state::down;
  switch (action) {
  case GLFW_PRESS:
    mouse_action = cr::mouse::button_state::down;
    break;
  case GLFW_RELEASE:
    mouse_action = cr::mouse::button_state::up;
    break;
  default:;
  }

  user_window->_mouse.buttons()[static_cast<int>(mouse_button)] = mouse_action;
}

void cr::display::_glfw_key_callback(GLFWwindow *window, int key, int scancode,
                                     int action, int mods) {
  auto user_window =
      static_cast<cr::display *>(glfwGetWindowUserPointer(window));

  if (action == GLFW_RELEASE)
    user_window->_keyboard.keys()[key].press_state =
        cr::keyboard::state::released;
  else if (action == GLFW_REPEAT)
    user_window->_keyboard.keys()[key].press_state =
        cr::keyboard::state::repeat;
  else if (action == GLFW_PRESS)
    user_window->_keyboard.keys()[key].press_state =
        cr::keyboard::state::pressed;

  user_window->_keyboard.keys()[key].mods = mods;
}

bool cr::display::key_down(cr::keyboard::key_code key) const noexcept {
  return _keyboard.is_key_down(key);
}

glm::ivec2 cr::display::mouse_position() const noexcept {
  return _mouse.position();
}
