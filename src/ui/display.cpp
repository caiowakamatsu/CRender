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
        if (pre_render_response.to_display.has_value())
        {
            glGenTextures(1, reinterpret_cast<GLuint *>(&_main_texture_handle));

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

        static auto dockspace_flags   = ImGuiDockNodeFlags_PassthruCentralNode;
        auto [window_flags, viewport] = IMGUI_init(dockspace_flags);

        ImGui::Begin("DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            IMGUI_setup_dock(dockspace_flags, window_flags, viewport);

        ImGui::End();

        IMGUI_scene_preview(pre_render_response, _main_texture_handle);

        const auto scene_hierarchy = IMGUI_scene_editor(pre_render_response.renderer);

        //        if (ImGui::CollapsingHeader("Material Editor"))
        //        {
        //            if (materials.has_value())
        //            {
        //                static auto current_material = -1;
        //
        //                if (ImGui::BeginCombo(
        //                      "Selected Material",
        //                      current_material == -1 ? ""
        //                                             :
        //                                             materials.value()[current_material].name().c_str()))
        //                {
        //                    for (auto i = 0; i < materials.value().size(); i++)
        //                    {
        //                        auto &material = materials.value()[i];
        //
        //                        auto is_selected =
        //                          current_material != -1 && material ==
        //                          materials.value()[current_material];
        //
        //                        if (ImGui::Selectable(material.name().c_str(), &is_selected))
        //                            current_material = i;
        //
        //                        if (is_selected) ImGui::SetItemDefaultFocus();
        //                    }
        //
        //                    ImGui::EndCombo();
        //                }
        //
        //                if (current_material != -1)
        //                {
        //                    ImGui::Indent(16.f);
        //                    auto &material = materials.value()[current_material];
        //
        //                    auto selected_material = static_cast<int>(material._material_type);
        //                    auto material_types    = std::vector<const char *>();
        //                    material_types.push_back("Metal");
        //                    material_types.push_back("Smooth");
        //
        //                    ImGui::Combo(
        //                      "Type",
        //                      &selected_material,
        //                      material_types.data(),
        //                      material_types.size());
        //
        //                    material._material_type =
        //                    static_cast<material::type>(selected_material);
        //
        //                    ImGui::SliderFloat3("RGB", glm::value_ptr(material._base_colour),
        //                    0.0f, 1.0f); ImGui::SliderFloat("Emission", &material._emission,
        //                    0.0f, 1.0f);
        //
        //                    switch (material.mat_type())
        //                    {
        //                    case material::metal:
        //                        ImGui::SliderFloat("Roughness", &material._roughness, 0.0f, 1.0f);
        //                        break;
        //
        //                    case material::smooth: break;
        //                    }
        //                    ImGui::Unindent(16.f);
        //                }
        //            }
        //
        //            if (ImGui::Button("Update Materials"))
        //            {
        //                auto renderer = pre_render_response.renderer->from_config(
        //                  pre_render_response.renderer->current_config());
        //
        //                renderer->scene()->update_materials(materials.value());
        //
        //                if (_updated_renderer_callback.has_value())
        //                    _updated_renderer_callback.value()(std::move(renderer));
        //            }
        //        }
        //
        //        if (ImGui::CollapsingHeader("Light Editor"))
        //        {
        //            auto lights = std::vector<const char *>();
        //            lights.emplace_back("Directional Light");
        //            lights.emplace_back("Point Light");
        //
        //            static auto current_light_type = 0;
        //            if (ImGui::BeginCombo("Create light", lights[current_light_type]))
        //            {
        //                for (auto i = 0; i < lights.size(); i++)
        //                {
        //                    auto is_selected = (current_light_type == i);
        //                    if (ImGui::Selectable(lights[i], &is_selected)) current_light_type =
        //                    i;
        //
        //                    if (is_selected) ImGui::SetItemDefaultFocus();
        //                }
        //                ImGui::EndCombo();
        //            }
        //
        //            ImGui::Indent(16.f);
        //            static auto strength  = 0.f;
        //            static auto colour    = glm::vec3();
        //            static auto position  = glm::vec3();
        //            static auto direction = glm::vec3();
        //
        //            switch (current_light_type)
        //            {
        //            case 0:
        //                ImGui::SliderFloat("Strength", &strength, 0, 15);
        //                ImGui::SliderFloat3("RGB", &colour[0], 0, 1);
        //                ImGui::InputFloat3("DIR", &direction[0]);
        //                break;
        //            case 1:
        //                ImGui::SliderFloat3("RGB", &colour[0], 0, 1);
        //                ImGui::InputFloat3("XYZ", &position[0]);
        //                ImGui::SliderFloat("Strength", &strength, 0, 15);
        //                break;
        //            default: break;
        //            }
        //
        //            if (ImGui::Button("Add light"))
        //            {
        //                auto renderer = pre_render_response.renderer->from_config(
        //                  pre_render_response.renderer->current_config());
        //
        //                if (current_light_type == 0)
        //                    renderer->scene()->_directional_lights.emplace_back(
        //                      direction,
        //                      colour,
        //                      strength);
        //                else if (current_light_type == 1)
        //                    renderer->scene()->_point_lights.emplace_back(position, colour,
        //                    strength);
        //
        //                if (_updated_renderer_callback.has_value())
        //                    _updated_renderer_callback.value()(std::move(renderer));
        //            }
        //            ImGui::Unindent(16.f);
        //        }
        //        ImGui::End();

        ImGui::Begin("General");

        static auto camera_position = glm::vec3(3, 2, 2);
        static auto camera_look_at  = glm::vec3(0, 0, 0);
        static auto fov             = 75.0f;

        static auto thread_count = std::thread::hardware_concurrency();

        static auto resolution = std::array<uint32_t, 2>({ 512, 512 });

        auto changed = false;
        if (ImGui::CollapsingHeader("Rendering"))
        {
            ImGui::InputFloat3("Position", &camera_position[0]);
            ImGui::InputFloat3("Look At", &camera_look_at[0]);
            ImGui::SliderFloat("FOV", &fov, 30.f, 120.f);
            ImGui::SliderInt(
              "Thread count",
              reinterpret_cast<int *>(&thread_count),
              1,
              static_cast<int>(std::thread::hardware_concurrency()));

            ImGui::InputInt2("Resolution (X, Y)", reinterpret_cast<int *>(&resolution[0]));
            for (auto &val : resolution) val = std::max(uint32_t(256), val);

            if (ImGui::Button("Update")) { changed = true; }
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

        ImGui::Render();
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        auto result = cr::display::post_render_result();
        if (changed && _postrender_callback.has_value())
        {
            auto config = cr::renderer::config();

            config.camera = cr::camera(
              camera_position,
              camera_look_at,
              fov,
              resolution[0] / static_cast<float>(resolution[1]));

            config.thread_count = thread_count;

            config.resolution.x = resolution[0];
            config.resolution.y = resolution[1];

            result.render_display_config = config;
        }
        _postrender_callback.value()(*this, result);
        glfwSwapBuffers(_glfw_window);
        glDeleteTextures(1, reinterpret_cast<const GLuint *>(&_main_texture_handle));
    }
    stop();
}

void cr::display::stop()
{
    glfwSetWindowShouldClose(_glfw_window, true);
    if (_close_callback.has_value()) _close_callback.value()(*this);
}

void cr::display::register_updated_renderer_callback(
  const std::function<void(std::unique_ptr<renderer>)> &callback)
{
    _updated_renderer_callback = callback;
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
  const std::function<void(cr::display &, post_render_result)> &callback)
{
    _postrender_callback = callback;
}
cr::display::ImGuiSetup cr::display::IMGUI_init(const ImGuiDockNodeFlags &dockspace_flags)
{
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

    ImGuiSetup value {};
    value.window_flags = window_flags;
    value.viewport     = viewport;
    return value;
}

void cr::display::IMGUI_setup_dock(
  const ImGuiDockNodeFlags &dockspace_flags,
  const ImGuiWindowFlags &  flags,
  const ImGuiViewport *     viewport)
{
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    static auto first_time = true;
    if (first_time)
    {
        first_time = false;

        ImGui::DockBuilderRemoveNode(dockspace_id);    // clear any previous layout
        ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

        auto dock_id_right =
          ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, nullptr, &dockspace_id);
        auto dock_id_bottom_left =
          ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.3f, nullptr, &dockspace_id);
        auto dock_id_top_left = ImGui::DockBuilderSplitNode(
          dock_id_bottom_left,
          ImGuiDir_Up,
          0.1f,
          nullptr,
          &dock_id_bottom_left);
        auto dock_id_middle = dockspace_id;

        ImGui::DockBuilderDockWindow("General", dock_id_right);
        ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_id_bottom_left);
        ImGui::DockBuilderDockWindow("Model Loader", dock_id_top_left);
        ImGui::DockBuilderDockWindow("Scene Preview", dock_id_middle);
        ImGui::DockBuilderFinish(dockspace_id);
    }
}

void cr::display::IMGUI_scene_preview(
  const cr::display::pre_render_response &pre_render_response,
  uint64_t                                texture_handle)
{
    ImGui::Begin("Scene Preview");
    if (pre_render_response.to_display.has_value())
        ImGui::Image(
          (void *) (intptr_t) texture_handle,
          ImVec2(
            pre_render_response.to_display->width(),
            pre_render_response.to_display->height()));
    ImGui::End();
}

cr::display::SceneEditorResponse cr::display::IMGUI_scene_editor(cr::renderer *renderer)
{
    static std::optional<std::vector<cr::material>> materials;

    ImGui::Begin("Scene Hierarchy");

    {
        const auto model = IMGUI_model_loader();
        if (model.has_value())
        {
            // Load the model into the scene
            auto new_renderer = std::make_unique<cr::renderer>(renderer->current_config());
            auto scene        = std::make_unique<cr::scene>(*renderer->scene());

            const auto model_data = cr::model::load_model(model.value().string());

            scene->add_triangles(model_data.vertices, model_data.vertex_indices);
            scene->add_materials(model_data.materials, model_data.material_indices);
            materials = model_data.materials;

            new_renderer->attach_scene(std::move(scene));

            if (_updated_renderer_callback.has_value())
                _updated_renderer_callback.value()(std::move(new_renderer));
        }
    }

    {
        ImGui::Indent(16.f);

        if (ImGui::CollapsingHeader("Model"))
            ;    // Empty for now

        if (ImGui::CollapsingHeader("Lights"))
            ;

        if (ImGui::CollapsingHeader("Material"))
        {
            // Display material list and return the selected one
            static auto previous_selected_index = -1;
            const auto  selected_material_index =
              IMGUI_material_list(renderer->scene()->loaded_materials());

            // If it's 0 we don't have a material selected
            if (selected_material_index != -1 && previous_selected_index != selected_material_index)
            {
                _selected_object.material =
                  &renderer->scene()->loaded_materials()[selected_material_index];
                _current_selected_type = _SelectedType::MATERIAL;
            }

            previous_selected_index = selected_material_index;
        }

        ImGui::Unindent(16.f);
    }

    ImGui::End();
    return cr::display::SceneEditorResponse();
}

std::optional<std::filesystem::path> cr::display::IMGUI_model_loader()
{
    static std::filesystem::path current_model;
    bool                         throw_away = false;

    ImGui::Begin("Model Loader");
    if (ImGui::BeginCombo("Select Model", current_model.filename().string().c_str()))
    {
        for (const auto &entry : std::filesystem::directory_iterator("./assets/models"))
        {
            if (
              entry.is_regular_file() &&
              ImGui::Selectable(entry.path().filename().string().c_str(), &throw_away))
            {
                current_model = entry;
                break;
            }
        }
        ImGui::EndCombo();
    }

    if (current_model != std::filesystem::path() && ImGui::Button("Load Model"))
    {
        ImGui::End();
        return current_model;
    }
    ImGui::End();

    return {};
}

int cr::display::IMGUI_material_list(const std::vector<cr::material> &material_list)
{
    static auto current_material = -1;

    if (material_list.empty()) current_material = -1;

    for (auto i = 0; i < material_list.size(); i++)
    {
        auto &material    = material_list[i];
        auto  is_selected = current_material != -1 && material == material_list[current_material];

        if (ImGui::Selectable(material.name().c_str(), &is_selected)) current_material = i;

        if (is_selected) ImGui::SetItemDefaultFocus();
    }

    return current_material;
}
