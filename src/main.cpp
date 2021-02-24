#include <string_view>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fmt/core.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>
#include <imgui/imnodes.h>

#include <ui/display.h>
#include <util/exception.h>
#include <ui/themes.h>

int main()
{
    auto main_display = Display();

    main_display.register_prerender_callback([](const Display &display){
        auto response = Display::PreRenderResponse();

        return response;
    });

    glfwSetErrorCallback([](int error, const char *description) {
        fmt::print("Error [{}], Description [{}]", error, description);
    });

    const auto init_glfw = glfwInit();
    if (!init_glfw) exit("Failed to initialize GLFW", 1);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    const auto window = glfwCreateWindow(1920, 1080, "CRender", nullptr, nullptr);
    if (!window) exit("Failed to create GLFW window", 2);

    glfwMakeContextCurrent(window);

    const auto load_gl = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    if (!load_gl) exit("Failed to load GLAD OpenGL functions", 20);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    imnodes::Initialize();

    auto &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGuiThemes::Red();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    auto image_handle = GLuint();
    auto image_width = int();
    auto image_height = int();
    // Load the image into the buffer and bind it to an opengl texture
    {
        auto image_data = stbi_load("./assets/rt-demo-mc.jpg", &image_width, &image_height, nullptr, 4);
        if (!image_data)
            exit("Couldn't load demo image, is it in the assets directory?", 2);

        glGenTextures(1, &image_handle);
        glBindTexture(GL_TEXTURE_2D, image_handle);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        stbi_image_free(image_data);
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
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

                ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
                ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

                auto dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, nullptr, &dockspace_id);
                auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.35f, nullptr, &dockspace_id);
                auto dock_id_top_left = dockspace_id;

                ImGui::DockBuilderDockWindow("Effect Editor", dock_id_down);
                ImGui::DockBuilderDockWindow("General", dock_id_right);
                ImGui::DockBuilderDockWindow("Scene Preview", dock_id_top_left);
                ImGui::DockBuilderFinish(dockspace_id);
            }
        }

        ImGui::End();

        ImGui::Begin("Scene Preview");
//        ImGui::Image((void*)(intptr_t) image_handle, ImVec2(image_width, image_height));
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

        glfwSwapBuffers(window);
    }

}
