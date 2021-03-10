#include "display.h"

cr::display::display()
{
    glfwSetErrorCallback([](int error, const char *description) {
        fmt::print("Error [{}], Description [{}]", error, description);
    });

    const auto init_glfw = glfwInit();
    if (!init_glfw) exit("Failed to initialize GLFW", 1);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    _glfw_window = glfwCreateWindow(1920, 1080, "CRender", nullptr, nullptr);
    if (!_glfw_window) exit("Failed to create GLFW window", 2);

    glfwMakeContextCurrent(_glfw_window);

    const auto load_gl = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    if (!load_gl) exit("Failed to load GLAD OpenGL functions", 20);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    imnodes::Initialize();
}

void cr::display::start()
{
    auto &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    cr::ImGuiThemes::Red();

    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *) _glfw_window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    while (!glfwWindowShouldClose(_glfw_window))
    {
        auto pre_render_response = _prerender_callback.value()(*this);
        if (pre_render_response.to_display != nullptr)
        {
            glGenTextures(1, &_main_texture_handle);

            glBindTexture(GL_TEXTURE_2D, _main_texture_handle);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(
              GL_TEXTURE_2D,
              0,
              GL_RGBA8,
              pre_render_response.to_display->width(),
              pre_render_response.to_display->height(),
              0,
              GL_RGBA,
              GL_UNSIGNED_BYTE,
              pre_render_response.to_display->data());
        }

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
          ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar(2);

        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

            static auto first_time = true;
            if (first_time)
            {
                first_time = false;

                ImGui::DockBuilderRemoveNode(dockspace_id);    // clear any previous layout
                ImGui::DockBuilderAddNode(
                  dockspace_id,
                  dockspace_flags | ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

                auto dock_id_right = ImGui::DockBuilderSplitNode(
                  dockspace_id,
                  ImGuiDir_Right,
                  0.2f,
                  nullptr,
                  &dockspace_id);
                auto dock_id_down = ImGui::DockBuilderSplitNode(
                  dockspace_id,
                  ImGuiDir_Down,
                  0.35f,
                  nullptr,
                  &dockspace_id);
                auto dock_id_top_left = dockspace_id;

                ImGui::DockBuilderDockWindow("Effect Editor", dock_id_down);
                ImGui::DockBuilderDockWindow("General", dock_id_right);
                ImGui::DockBuilderDockWindow("Scene Preview", dock_id_top_left);
                ImGui::DockBuilderFinish(dockspace_id);
            }
        }

        ImGui::End();

        ImGui::Begin("Scene Preview");
        if (pre_render_response.to_display != nullptr)
            ImGui::Image(
              (void *) (intptr_t) _main_texture_handle,
              ImVec2(
                pre_render_response.to_display->width(),
                pre_render_response.to_display->height()));
        ImGui::End();

        ImGui::Begin("General");

        if (ImGui::CollapsingHeader("Replay Mod File Loader"))
        {
            // Todo: Loader for Replay mod
        }

        if (ImGui::CollapsingHeader("Output Settings"))
        {
            static int resolution[2];
            ImGui::InputInt2("Resolution", &resolution[0]);
            static int fps;
            ImGui::InputInt("Frames Per Seconds (FPS)", &fps);
            ImGui::NewLine();
            ImGui::Button("Render");
        }

        ImGui::End();

        ImGui::Begin("Effect Editor");
        imnodes::BeginNodeEditor();
        imnodes::EndNodeEditor();
        ImGui::End();

        ImGui::Render();
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //        const auto post_render_result = _postrender_callback.value()(*this);
        glfwSwapBuffers(_glfw_window);
        glDeleteTextures(1, &_main_texture_handle);
    }
}

void cr::display::stop()
{
    glfwSetWindowShouldClose(_glfw_window, true);
}

void cr::display::register_close_callback(const std::function<void(cr::display &)> &callback)
{
    _close_callback = callback;
}

void cr::display::register_prerender_callback(
  const std::function<pre_render_response(cr::display &)> &callback)
{
    _prerender_callback = callback;
}

void cr::display::register_postrender_callback(
  const std::function<post_render_result(cr::display &)> &callback)
{
    _postrender_callback = callback;
}