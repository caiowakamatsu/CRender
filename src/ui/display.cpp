#include "display.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stbi_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

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

    glGenTextures(1, &_scene_texture_handle);
    glGenTextures(1, &_target_texture);

    glfwSetWindowUserPointer(_glfw_window, this);

    glfwSetCursorPosCallback(_glfw_window, [](GLFWwindow *window, double x, double y) {
        auto ptr = reinterpret_cast<display *>(glfwGetWindowUserPointer(window));

        ptr->_mouse_pos.x = x;
        ptr->_mouse_pos.y = y;

        ptr->_mouse_change_prev = ptr->_mouse_pos_prev - ptr->_mouse_pos;

        // coordinates are reversed on y axis (top left vs bottom left)
        ptr->_mouse_change_prev.x *= -1;

        ptr->_mouse_pos_prev = ptr->_mouse_pos;
    });

    glfwSetKeyCallback(
      _glfw_window,
      [](GLFWwindow *window, int key, int scancode, int action, int mods) {
          auto ptr = reinterpret_cast<display *>(glfwGetWindowUserPointer(window));

          if (action == GLFW_RELEASE)
              ptr->_key_states[key] = key_state::released;
          else if (action == GLFW_REPEAT)
              ptr->_key_states[key] = key_state::repeat;
          else if (action == GLFW_PRESS)
              ptr->_key_states[key] = key_state::pressed;
      });
}

void cr::display::start(
  std::unique_ptr<cr::scene> *         scene,
  std::unique_ptr<cr::renderer> *      renderer,
  std::unique_ptr<cr::draft_renderer> *draft_renderer,
  std::unique_ptr<cr::thread_pool> *   thread_pool)
{
    auto work_group_max = std::array<int, 3>();

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_group_max[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_group_max[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_group_max[2]);
    fmt::print(
      "Maximum compute work group count [x: {}, y: {}, z: {}]\n",
      work_group_max[0],
      work_group_max[1],
      work_group_max[2]);

    auto &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    cr::ImGuiThemes::CorporateGrey();

    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *) _glfw_window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    bool draft_mode_changed = false;
    while (!glfwWindowShouldClose(_glfw_window))
    {
        _timer.frame_start();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto ui_ctx = cr::ui::init();

        if (!_in_draft_mode && draft_mode_changed)
        {
            draft_mode_changed = false;
            renderer->get()->start();
        }
        else if (_in_draft_mode && draft_mode_changed)
        {
            draft_mode_changed = false;
            renderer->get()->pause();
        }

        // Root imgui node (Not visible)
        ui::root_node(ui_ctx);

        ui::render_quality(renderer->get());

        {
            // Render export frame
            ImGui::Begin("Export");

            static auto file_string = std::array<char, 32>();
            ImGui::InputTextWithHint("File Name (JPG)", "Max 32 chars", file_string.data(), 32);

            if (ImGui::Button("Save"))
            {
                // Checking if the file already exists
                auto directory      = std::string("./out/") + file_string.data() + ".jpg";
                auto attempt_number = 1;
                while (std::filesystem::exists(directory))
                    directory = std::string("./out/") + file_string.data() + ' ' + "(" +
                      std::to_string(attempt_number++) + ")" + ".jpg";

                const auto data = renderer->get()->current_progress();
                stbi_write_jpg(
                  directory.c_str(),
                  data->width(),
                  data->height(),
                  4,
                  data->data(),
                  100);
            }

            ImGui::End();
        }

        {
            ImGui::Begin("Skybox Loader");

            static std::string current_skybox;
            bool               throw_away = false;

            if (ImGui::BeginCombo("Select Skybox", current_skybox.c_str()))
            {
                for (const auto &entry : std::filesystem::directory_iterator("./assets/skybox"))
                {
                    if (entry.is_directory()) continue;

                    if (ImGui::Selectable(entry.path().filename().string().c_str(), &throw_away))
                    {
                        current_skybox = entry.path().string();
                        break;
                    }
                }
                ImGui::EndCombo();
            }

            if (!current_skybox.empty() && ImGui::Button("Load Skybox"))
            {
                // Load skybox in
                renderer->get()->update([&scene, current_skybox = current_skybox] {
                    auto image = cr::asset_loader::load_picture(current_skybox);

                    auto skybox = cr::image(image.colour, image.res.x, image.res.y);

                    scene->get()->set_skybox(std::move(skybox));
                });
            }

            ImGui::End();
        }

        {
            // Render model loader
            ImGui::Begin("Model Loader");

            static std::string current_directory;
            static std::string current_model;
            bool               throw_away = false;

            if (ImGui::BeginCombo("Select Model", current_directory.c_str()))
            {
                for (const auto &entry : std::filesystem::directory_iterator("./assets/models"))
                {
                    if (!entry.is_directory()) continue;

                    const auto model_path = cr::asset_loader::valid_directory(entry);
                    if (!model_path.has_value()) continue;

                    // Go through each file in the directory
                    if (ImGui::Selectable(entry.path().filename().string().c_str(), &throw_away))
                    {
                        current_directory = entry.path().string();
                        current_model     = model_path.value();
                        break;
                    }
                }
                ImGui::EndCombo();
            }

            if (current_model != std::filesystem::path() && ImGui::Button("Load Model"))
            {
                // Load model in
                const auto model_data = cr::asset_loader::load_model(current_model, current_directory);

                if (!_in_draft_mode)
                    renderer->get()->update(
                      [&scene, &model_data] { scene->get()->add_model(model_data); });
                else
                    scene->get()->add_model(model_data);
            }

            ImGui::End();
        }

        {
            // Render bottom left thing
            ImGui::Begin("Scene Objects");

            auto &registry = scene->get()->registry()->entities;

            static auto index = 0;
            registry.each([&registry, &index = index, this](auto entity) {
                static auto selected = 0;
                if (ImGui::RadioButton(registry.get<std::string>(entity).c_str(), &selected, index))
                    _current_entity = entity;
                index++;
            });
            index = 0;

            ImGui::End();
        }

        {
            // Render middle thing
            ImGui::Begin("Scene Preview");

            auto window_size = ImGui::GetContentRegionAvail();

            if (_in_draft_mode)
            {
                draft_renderer->get()->render();
                ImGui::Image((void *) draft_renderer->get()->rendered_texture(), window_size);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, _target_texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexImage2D(
                  GL_TEXTURE_2D,
                  0,
                  GL_RGBA8,
                  static_cast<int>(window_size.x),
                  static_cast<int>(window_size.y),
                  0,
                  GL_RGBA,
                  GL_UNSIGNED_BYTE,
                  nullptr);
                glBindImageTexture(0, _target_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

                const auto current_progress = renderer->get()->current_progress();
                // Upload rendered scene to GPU
                glBindTexture(GL_TEXTURE_2D, _scene_texture_handle);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                glTexImage2D(
                  GL_TEXTURE_2D,
                  0,
                  GL_RGBA8,
                  current_progress->width(),
                  current_progress->height(),
                  0,
                  GL_RGBA,
                  GL_FLOAT,
                  current_progress->data());
                glActiveTexture(GL_TEXTURE1);

                // Set uniforms
                {
                    static auto current_translation = glm::vec2(0.0f, 0.0f);
                    static auto current_zoom        = float(1);
                    if (ImGui::IsWindowHovered())
                    {
                        current_zoom += io.MouseWheel * -.05;

                        if (ImGui::IsMouseDown(0))
                        {
                            const auto delta =
                              glm::vec2(io.MouseDelta.x, io.MouseDelta.y) * glm::vec2(-1, -1);
                            current_translation.x += delta.x;
                            current_translation.y += delta.y;
                        }
                    }

                    glUniform2fv(
                      glGetUniformLocation(_compute_shader_program, "translation"),
                      1,
                      glm::value_ptr(current_translation));

                    glUniform1f(
                      glGetUniformLocation(_compute_shader_program, "zoom"),
                      current_zoom);
                }

                {
                    glUseProgram(_compute_shader_program);

                    glDispatchCompute(
                      static_cast<int>(window_size.x),
                      static_cast<int>(window_size.y),
                      1);

                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                }
                ImGui::Image((void *) _scene_texture_handle, window_size);
            }

            ImGui::End();
        }

        {
            // Render right thing
            ImGui::Begin("Property Editor");

            if (_current_entity.has_value())
            {
                auto       should_update = false;
                auto &     registry      = scene->get()->registry()->entities;
                const auto current_type  = registry.get<cr::entity::type>(_current_entity.value());
                auto       should_reset  = false;
                {
                    static auto current_entity = std::optional<uint64_t>();
                    if (
                      !current_entity.has_value() ||
                      current_entity.value() != _current_entity.value())
                    {
                        should_reset   = true;
                        current_entity = _current_entity.value();
                    }
                }

                if (ImGui::Button("Update"))
                {
                    renderer->get()->pause();
                    should_update = true;
                }

                ImGui::Indent(16.f);

                if (current_type == cr::entity::type::CAMERA)
                {
                    const auto &camera = registry.get<cr::camera>(_current_entity.value());

                    static auto position = std::optional<glm::vec3>();
                    if (!position.has_value()) position = camera.position;
                    static auto look_at = std::optional<glm::vec3>();
                    //                    if (!look_at.has_value()) look_at = camera.look_at;
                    static auto fov = std::optional<float>();
                    if (!fov.has_value()) fov = camera.fov;

                    ImGui::InputFloat3("Position", glm::value_ptr(position.value()));
                    //                    ImGui::InputFloat3("Look At",
                    //                    glm::value_ptr(look_at.value()));
                    ImGui::SliderFloat("FOV", &fov.value(), 10, 120);

                    if (should_update)
                    {
                        //                        registry.get<cr::camera>(_current_entity.value())
                        //                        =
                        //                          cr::camera(position.value(), look_at.value(),
                        //                          fov.value());

                        position.reset();
                        look_at.reset();
                        fov.reset();
                    }
                }
                else if (current_type == cr::entity::type::MODEL)
                {
                    // This code initializes the value only once with the current model
                    // Transformations
                    static auto transforms = std::optional<std::vector<glm::mat4>>();
                    if (!transforms.has_value() || should_reset)
                    {
                        transforms =
                          registry.get<cr::entity::transforms>(_current_entity.value()).data;
                    }

                    // Materials
                    static auto materials = std::optional<cr::entity::model_materials>();
                    if (!materials.has_value() || should_reset)
                    {
                        materials =
                          registry.get<cr::entity::model_materials>(_current_entity.value());
                    }

                    // Editor
                    if (ImGui::CollapsingHeader("Transformations"))
                    {
                        ImGui::Indent(16.f);
                        for (auto &transform : transforms.value())
                        {
                            static auto position = glm::vec3(0, 0, 0);
                            static auto scale    = glm::vec3(1, 1, 1);
                            ImGui::InputFloat3("Position##Camera", glm::value_ptr(position));
                            ImGui::InputFloat3("Scale##Camera", glm::value_ptr(scale));

                            transform = glm::translate(transform, -position);
                            transform = glm::scale(transform, scale);
                        }
                        ImGui::Unindent(16.f);
                    }

                    if (ImGui::CollapsingHeader("Materials"))
                    {
                        ImGui::Indent(16.f);
                        for (auto &material : materials.value().materials)
                        {
                            ImGui::Text("%s", material.info.name.c_str());
                            ImGui::Indent(16.f);

                            // Light type
                            const char *       items[]      = { "Metal", "Smooth" };
                            static const char *current_item = nullptr;

                            switch (material.info.type)
                            {
                            case cr::material::type::metal: current_item = items[0]; break;

                            case cr::material::type::smooth: current_item = items[1]; break;
                            }

                            if (ImGui::BeginCombo(
                                  (std::string("Type") + "##" + material.info.name).c_str(),
                                  current_item))    // The second parameter is the label
                                                    // previewed before opening the combo.
                            {
                                for (auto &item : items)
                                {
                                    bool is_selected =
                                      (current_item ==
                                       item);    // You can store your selection however you
                                                 // want, outside or inside your objects
                                    if (ImGui::Selectable(item, is_selected)) current_item = item;
                                    if (is_selected)
                                        ImGui::SetItemDefaultFocus();    // You may set the
                                                                         // initial focus when
                                                                         // opening the combo
                                                                         // (scrolling + for
                                                                         // keyboard navigation
                                                                         // support)
                                }
                                ImGui::EndCombo();
                            }

                            if (current_item == items[0])
                                material.info.type = cr::material::type::metal;
                            else if (current_item == items[1])
                                material.info.type = cr::material::type::smooth;

                            ImGui::SliderFloat(
                              (std::string("IOR") + "##" + material.info.name).c_str(),
                              &material.info.ior,
                              0,
                              1);
                            ImGui::SliderFloat(
                              (std::string("Roughness") + "##" + material.info.name).c_str(),
                              &material.info.roughness,
                              0,
                              1);
                            ImGui::SliderFloat(
                              (std::string("Emission") + "##" + material.info.name).c_str(),
                              &material.info.emission,
                              0,
                              50);
                            ImGui::ColorEdit3(
                              (std::string("Colour") + "##" + material.info.name).c_str(),
                              glm::value_ptr(material.info.colour));
                            ImGui::Unindent(16.f);
                        }
                        ImGui::Unindent(16.f);
                    }

                    if (should_update)
                    {
                        // Update the value
                        registry.get<cr::entity::transforms>(_current_entity.value()).data =
                          transforms.value();
                        registry.get<cr::entity::model_materials>(_current_entity.value()) =
                          materials.value();

                        transforms.reset();
                        materials.reset();
                    }
                }

                ImGui::Unindent(16.f);
                if (should_update) renderer->get()->start();
            }

            ImGui::End();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ImGui::Render();
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        _timer.frame_stop();
        glfwSwapBuffers(_glfw_window);

        if (_key_states[static_cast<int>(key_code::KEY_R)] == key_state::pressed)
        {
            _in_draft_mode     = !_in_draft_mode;
            draft_mode_changed = true;
        }

        glfwSetInputMode(
          _glfw_window,
          GLFW_CURSOR,
          _in_draft_mode ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

        if (_in_draft_mode) _update_camera(scene->get()->registry()->camera());
        _poll_events();
    }
    glDeleteTextures(1, &_scene_texture_handle);
    stop();
}

void cr::display::_poll_events()
{
    for (auto &key : _key_states)
    {
        if (key == key_state::released) key = key_state::none;
        if (key == key_state::pressed) key = key_state::held;
    }
    glfwPollEvents();
}

void cr::display::stop()
{
    glfwSetWindowShouldClose(_glfw_window, true);
}

cr::display::~display()
{
    glDeleteTextures(1, &_target_texture);
    glDeleteTextures(1, &_scene_texture_handle);
    glDeleteShader(_compute_shader_id);
    glDeleteProgram(_compute_shader_program);
}

void cr::display::_update_camera(cr::camera *camera)
{
    auto translation = glm::vec3();
    auto rotation    = glm::vec3();

    if (
      _key_states[static_cast<int>(key_code::SPACE)] == key_state::held ||
      _key_states[static_cast<int>(key_code::SPACE)] == key_state::repeat)
        translation.y += 3.0f;

    if (
      _key_states[static_cast<int>(key_code::KEY_LEFT_CONTROL)] == key_state::held ||
      _key_states[static_cast<int>(key_code::KEY_LEFT_CONTROL)] == key_state::repeat)
        translation.y -= 3.0f;

    if (
      _key_states[static_cast<int>(key_code::KEY_W)] == key_state::held ||
      _key_states[static_cast<int>(key_code::KEY_W)] == key_state::repeat)
        translation.z -= 3.0f;

    if (
      _key_states[static_cast<int>(key_code::KEY_S)] == key_state::held ||
      _key_states[static_cast<int>(key_code::KEY_S)] == key_state::repeat)
        translation.z += 3.0f;

    if (
      _key_states[static_cast<int>(key_code::KEY_D)] == key_state::held ||
      _key_states[static_cast<int>(key_code::KEY_D)] == key_state::repeat)
        translation.x -= 3.0f;

    if (
      _key_states[static_cast<int>(key_code::KEY_A)] == key_state::held ||
      _key_states[static_cast<int>(key_code::KEY_A)] == key_state::repeat)
        translation.x += 3.0f;

    translation *= static_cast<float>(_timer.since_last_frame()) * 5.75f;

    camera->translate(translation);

    rotation.x += _mouse_change_prev.x * 2.0 * _timer.since_last_frame();
    rotation.y += _mouse_change_prev.y * 2.0 * _timer.since_last_frame();

    _mouse_change_prev = {};

    camera->rotate(rotation);
}
