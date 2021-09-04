#pragma once

#include <string>

#include <imgui/imgui.h>
#include <render/renderer.h>
#include <render/timer.h>
#include <render/draft/draft_renderer.h>
#include <util/asset_loader.h>
#include <util/algorithm.h>
#include <util/denoise.h>
#include <stb/stbi_image_write.h>
#include <stb/stb_image.h>
#include <render/post/post_processor.h>
#include "display.h"

namespace cr
{
    enum key_code
    {
        SPACE             = 32,    // I can't wait to write all of these out
        APOSTROPHE        = 39,
        COMMA             = 44,
        MINUS             = 45,
        PERIOD            = 46,
        SLASH             = 47,
        KEY_0             = 48,
        KEY_1             = 49,
        KEY_2             = 50,
        KEY_3             = 51,
        KEY_4             = 52,
        KEY_5             = 53,
        KEY_6             = 54,
        KEY_7             = 55,
        KEY_8             = 56,
        KEY_9             = 57,    // This is starting to get really annoying
        SEMICOLON         = 59,
        EQUAL             = 61,
        KEY_A             = 65,
        KEY_B             = 66,
        KEY_C             = 67,
        KEY_D             = 68,
        KEY_E             = 69,
        KEY_F             = 70,
        KEY_G             = 71,
        KEY_H             = 72,
        KEY_I             = 73,
        KEY_J             = 74,
        KEY_K             = 75,
        KEY_L             = 76,
        KEY_M             = 77,
        KEY_N             = 78,
        KEY_O             = 79,
        KEY_P             = 80,
        KEY_Q             = 81,
        KEY_R             = 82,
        KEY_S             = 83,
        KEY_T             = 84,
        KEY_U             = 85,
        KEY_V             = 86,
        KEY_W             = 87,
        KEY_X             = 88,
        KEY_Y             = 89,
        KEY_Z             = 90,
        KEY_LEFT_BRACKET  = 91,  /* [ */
        KEY_BACKSLASH     = 92,  /* \ */
        KEY_RIGHT_BRACKET = 93,  /* ] */
        KEY_GRAVE_ACCENT  = 96,  /* ` */
        KEY_WORLD_1       = 161, /* non-US #1 */
        KEY_WORLD_2       = 162, /* non-US #2 */
        KEY_ESCAPE        = 256,
        KEY_ENTER         = 257,
        KEY_TAB           = 258,
        KEY_BACKSPACE     = 259,
        KEY_INSERT        = 260,
        KEY_DELETE        = 261,
        KEY_RIGHT         = 262,
        KEY_LEFT          = 263,
        KEY_DOWN          = 264,
        KEY_UP            = 265,
        KEY_PAGE_UP       = 266,
        KEY_PAGE_DOWN     = 267,
        KEY_HOME          = 268,
        KEY_END           = 269,
        KEY_CAPS_LOCK     = 280,
        KEY_SCROLL_LOCK   = 281,
        KEY_NUM_LOCK      = 282,
        KEY_PRINT_SCREEN  = 283,
        KEY_PAUSE         = 284,
        KEY_F1            = 290,
        KEY_F2            = 291,
        KEY_F3            = 292,
        KEY_F4            = 293,
        KEY_F5            = 294,
        KEY_F6            = 295,
        KEY_F7            = 296,
        KEY_F8            = 297,
        KEY_F9            = 298,
        KEY_F10           = 299,
        KEY_F11           = 300,
        KEY_F12           = 301,
        KEY_F13           = 302,
        KEY_F14           = 303,
        KEY_F15           = 304,
        KEY_F16           = 305,
        KEY_F17           = 306,
        KEY_F18           = 307,
        KEY_F19           = 308,
        KEY_F20           = 309,
        KEY_F21           = 310,
        KEY_F22           = 311,
        KEY_F23           = 312,
        KEY_F24           = 313,
        KEY_F25           = 314,
        KEY_KP_0          = 320,
        KEY_KP_1          = 321,
        KEY_KP_2          = 322,
        KEY_KP_3          = 323,
        KEY_KP_4          = 324,
        KEY_KP_5          = 325,
        KEY_KP_6          = 326,
        KEY_KP_7          = 327,
        KEY_KP_8          = 328,
        KEY_KP_9          = 329,
        KEY_KP_DECIMAL    = 330,
        KEY_KP_DIVIDE     = 331,
        KEY_KP_MULTIPLY   = 332,
        KEY_KP_SUBTRACT   = 333,
        KEY_KP_ADD        = 334,
        KEY_KP_ENTER      = 335,
        KEY_KP_EQUAL      = 336,
        KEY_LEFT_SHIFT    = 340,
        KEY_LEFT_CONTROL  = 341,
        KEY_LEFT_ALT      = 342,
        KEY_LEFT_SUPER    = 343,
        KEY_RIGHT_SHIFT   = 344,
        KEY_RIGHT_CONTROL = 345,
        KEY_RIGHT_ALT     = 346,
        KEY_RIGHT_SUPER   = 347,
        KEY_MENU          = 348,
        MAX_KEY           = 1024,
    };

    enum class key_state
    {
        none,       // no input
        pressed,    // set on frame key is pressed
        held,       // set on frame after key is pressed while not released
        repeat,     // equivalent to held but OS triggered
        released    // set on frame key is released, cleared after one frame
    };
}    // namespace cr

namespace cr::ui
{
    namespace widgets
    {
        inline bool slider_float_input(const std::string &label, float &value, float min, float max)
        {
            // Default is the slider
            static auto text_labels = std::unordered_set<std::string>();

            const auto is_text = text_labels.find(label) != text_labels.end();

            auto updated = false;
            if (!is_text)
                updated = ImGui::SliderFloat(label.c_str(), &value, min, max);
            else
            {
                updated = ImGui::InputFloat(label.c_str(), &value);
                value   = glm::min(value, max);
                value   = glm::max(value, min);
            }

            // check if it's a double click
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                if (is_text)
                    text_labels.erase(label);
                else
                    text_labels.insert(label);
            return updated;
        }
    }    // namespace widgets

    struct init_ctx
    {
        ImGuiDockNodeFlags dock_flags;
        ImGuiWindowFlags   window_flags;
        ImGuiViewport *    viewport;
    };
    [[nodiscard]] inline init_ctx init()
    {
        auto dock_flags   = ImGuiDockNodeFlags_PassthruCentralNode;
        auto window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        auto viewport     = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
          ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        if (dock_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        auto ctx         = init_ctx();
        ctx.dock_flags   = dock_flags;
        ctx.window_flags = window_flags;
        ctx.viewport     = viewport;
        return ctx;
    }

    inline void init_dock(
      const init_ctx &   ctx,
      const std::string &top_left,
      const std::string &bottom_left,
      const std::string &right_panel)
    {
        auto dockspace_id = ImGui::GetID("DockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ctx.dock_flags);

        static auto first_time = true;
        if (first_time)
        {
            first_time = false;

            ImGui::DockBuilderRemoveNode(dockspace_id);    // clear any previous layout
            ImGui::DockBuilderAddNode(dockspace_id, ctx.dock_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ctx.viewport->Size);

            auto dock_id_right = ImGui::DockBuilderSplitNode(
              dockspace_id,
              ImGuiDir_Right,
              0.2f,
              nullptr,
              &dockspace_id);

            auto dock_id_down = ImGui::DockBuilderSplitNode(
              dockspace_id,
              ImGuiDir_Down,
              0.2f,
              nullptr,
              &dockspace_id);

            ImGui::DockBuilderGetNode(dock_id_right)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
            ImGui::DockBuilderGetNode(dock_id_down)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
            ImGui::DockBuilderGetNode(dockspace_id)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

            ImGui::DockBuilderDockWindow(top_left.c_str(), dockspace_id);
            ImGui::DockBuilderDockWindow(right_panel.c_str(), dock_id_right);
            ImGui::DockBuilderDockWindow(bottom_left.c_str(), dock_id_down);

            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    inline void root_node(init_ctx ui_ctx)
    {
        ImGui::Begin("DockSpace", nullptr, ui_ctx.window_flags);
        ImGui::PopStyleVar(3);

        // Setup the dock
        cr::ui::init_dock(ui_ctx, "Scene Preview", "Console", "Misc");

        ImGui::End();
    }

    inline void scene_preview(
      cr::renderer *      renderer,
      cr::draft_renderer *draft_renderer,
      cr::scene *         scene,
      GLuint              target_texture,
      GLuint              scene_texture,
      GLuint              compute_program,
      bool                in_draft_mode)
    {
        ImGui::Begin("Scene Preview");
        auto window_size = ImGui::GetContentRegionAvail();

        // Set uniforms
        {
            auto        io                  = ImGui::GetIO();
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

            glUseProgram(compute_program);

            glUniform1f(glGetUniformLocation(compute_program, "zoom"), current_zoom);

            glUniform2fv(
              glGetUniformLocation(compute_program, "translation"),
              1,
              glm::value_ptr(current_translation));

            glUniform2i(
              glGetUniformLocation(compute_program, "target_size"),
              window_size.x,
              window_size.y);

            glUniform2i(
              glGetUniformLocation(compute_program, "scene_size"),
              renderer->current_resolution().x,
              renderer->current_resolution().y);

            glUniformMatrix4fv(
              glGetUniformLocation(compute_program, "camera"),
              1,
              GL_FALSE,
              glm::value_ptr(scene->registry()->camera()->mat4()));

            glUniform1i(glGetUniformLocation(compute_program, "flip"), in_draft_mode);
        }

        {
            if (in_draft_mode) draft_renderer->render();
            glUseProgram(compute_program);

            glBindTexture(GL_TEXTURE_2D, target_texture);
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
            glClearTexImage(target_texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glBindImageTexture(0, target_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

            if (in_draft_mode)
            {
                const auto rendered = draft_renderer->rendered_texture();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, rendered);
            }
            else
            {
                const auto current_progress = renderer->current_progress();
                // Upload rendered scene to GPU
                glBindTexture(GL_TEXTURE_2D, scene_texture);
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
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, scene_texture);
            }

            glDispatchCompute(
              static_cast<int>(glm::ceil(window_size.x / 8)),
              static_cast<int>(glm::ceil(window_size.y / 8)),
              1);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
        ImGui::Image((void *) target_texture, window_size);
        ImGui::End();
    }

    inline void setting_render(
      cr::renderer *                    renderer,
      cr::draft_renderer *              draft_renderer,
      cr::scene *                       scene,
      std::unique_ptr<cr::thread_pool> &pool,
      glm::vec2 &speed_multipliers)
    {
        static auto resolution   = glm::ivec2();
        static auto bounces      = int(5);
        static auto thread_count = static_cast<int>(std::thread::hardware_concurrency());

        if (resolution.x == 0) resolution.x = renderer->current_resolution().x;

        if (resolution.y == 0) resolution.y = renderer->current_resolution().y;

        ImGui::InputInt2("Resolution", glm::value_ptr(resolution));
        ImGui::InputInt("Max Bounces (?)", &bounces);
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();

            ImGui::Text("How many times can the ray bounce around before getting terminated");
            ImGui::Separator();
            ImGui::Text("Suggested amounts");
            ImGui::Text("---------");
            ImGui::Text("< 4 | Not suggested");
            ImGui::Text("5-12 | Good for general use");
            ImGui::Text("12-20 | Enough for near perfect lighting");
            ImGui::Text("> 20 | Not suggested, performance to lighting tradeoff not optimal");
            ImGui::EndTooltip();
        }
        ImGui::InputInt("Thread Count", &thread_count);

        if (ImGui::Button("Update"))
        {
            renderer->update(
              [renderer, draft_renderer, &pool]()
              {
                  renderer->set_max_bounces(bounces);
                  renderer->set_resolution(resolution.x, resolution.y);
                  draft_renderer->set_resolution(resolution.x, resolution.y);
                  pool = std::make_unique<cr::thread_pool>(thread_count);
              });
        }

        ImGui::Text(
          "%s",
          fmt::format("Current sample count: [{}]", renderer->current_sample_count()).c_str());

        ImGui::NewLine();
        ImGui::Separator();

        if (ImGui::Button("Pause"))
        {
            const auto success = renderer->pause();

            if (success)
                cr::logger::info("Paused the renderer successfully");
            else
                cr::logger::warn("Cannot pause the renderer when it's already paused");
        }
        else if (ImGui::SameLine(); ImGui::Button("Start"))
        {
            const auto success = renderer->start();

            if (success)
                cr::logger::info("Started the renderer successfully");
            else
                cr::logger::warn("Cannot start the renderer when it's started");
        }
        static auto target_spp = int(0);
        ImGui::Text("Target Sample Count (?)");
        ImGui::InputInt("Count", &target_spp, 16, 64);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Set amount of samples per pixel you want to render, 0 for no limit");
        if (ImGui::Button("Set target sample count")) renderer->set_target_spp(target_spp);

        {
            ImGui::Text("Sun");
            ImGui::Indent(4.f);

            static auto sun_enabled = true;
            static auto sun         = cr::entity::sun();
            ImGui::InputFloat("Sun Size", &sun.size);
            sun.size = glm::max(sun.size, 0.1f);

            ImGui::InputFloat("Intensity", &sun.intensity);
            ImGui::Checkbox("Sun enabled", &sun_enabled);
            ImGui::ColorEdit3("Colour", glm::value_ptr(sun.colour));

            static auto sun_dir = glm::vec3(0.8, -1, 0.0);

            static auto use_angles = false;
            ImGui::Checkbox("Use Angles", &use_angles);
            if (use_angles)
            {
                ImGui::SliderFloat("Elevation", &sun_dir.x, -90, 90);
                ImGui::SliderFloat("Azimuth", &sun_dir.y, 0, 360);
            }
            else
            {
                ImGui::InputFloat("Direction X", &sun_dir.x);
                ImGui::InputFloat("Direction Y", &sun_dir.y);
                ImGui::InputFloat("Direction Z", &sun_dir.z);
            }

            if (ImGui::Button("Update Sun"))
            {
                const auto elevation = -sun_dir.x / 360.f * cr::numbers<float>::tau;
                const auto azimuth   = sun_dir.y / 360.f * cr::numbers<float>::tau;

                if (use_angles)
                    sun.direction = glm::normalize(glm::vec3(
                      glm::cos(azimuth) * glm::cos(elevation),
                      glm::sin(elevation),
                      glm::sin(azimuth) * glm::sin(elevation)));
                else
                    sun.direction = glm::normalize(glm::vec3(sun_dir.x, sun_dir.y, sun_dir.z));

                renderer->update(
                  [sun_enabled = sun_enabled, sun = sun, scene]
                  {
                      scene->registry()->set_sun(sun);
                      scene->set_sun_enabled(sun_enabled);
                  });
            }

            ImGui::Unindent(4.f);
        }

        {
            ImGui::Separator();
            ImGui::Text("Input Sensitivity (Draft Mode)");
            ImGui::Indent(4.f);

            ImGui::SliderFloat("Movement (Keyboard)", &speed_multipliers.x, 0.1f, 20.f);
            ImGui::SliderFloat("Rotation (Mouse)", &speed_multipliers.y, 0.1f, 20.f);

            ImGui::Unindent(4.f);
        }
    }

    inline void setting_export(
      std::unique_ptr<cr::renderer> *      renderer,
      std::unique_ptr<cr::post_processor> *processor)
    {
        static auto file_string = std::array<char, 32>();
        ImGui::InputTextWithHint("File Name", "Max 32 chars", file_string.data(), 64);

        static const auto export_types = std::array<std::string, 4>({ "PNG", "JPG", "EXR", "HDR" });

        static auto current_type = 0;

        if (ImGui::BeginCombo("Export Type", export_types[current_type].c_str()))
        {
            for (auto i = 0; i < export_types.size(); i++)
                if (ImGui::Button(export_types[i].c_str())) current_type = i;
            ImGui::EndCombo();
        }

        auto selected_type = cr::asset_loader::image_type::PNG;

        switch (current_type)
        {
        case 0: selected_type = asset_loader::image_type::PNG; break;
        case 1: selected_type = asset_loader::image_type::JPG; break;
        case 2: selected_type = asset_loader::image_type::EXR; break;
        case 3: selected_type = asset_loader::image_type::HDR; break;
        }

        static auto export_albedo = false;
        static auto export_normal = false;
        static auto export_depth  = false;
        static auto denoise       = true;
        static auto post_process  = true;
        ImGui::Checkbox("Export Albedo", &export_albedo);
        ImGui::Checkbox("Export Normal", &export_normal);
        ImGui::Checkbox("Export Depth", &export_depth);
        ImGui::Checkbox("Denoise", &denoise);
        ImGui::Checkbox("Post Process", &post_process);

        if (ImGui::Button("Save"))
        {
            cr::logger::info("Starting to export image [{}]", file_string.data());
            auto timer = cr::timer();

            auto file_str = std::string(file_string.data());

            const auto data = renderer->get()->current_progress();

            auto folder = export_albedo || export_normal || export_depth || denoise || post_process;

            if (folder)
            {
                std::filesystem::create_directories("./out/" + file_str);
                file_str = file_str + "\\sample";
            }

            cr::asset_loader::export_framebuffer(*data, file_str.data(), selected_type);

            if (export_albedo)
                cr::asset_loader::export_framebuffer(
                  *renderer->get()->current_albedos(),
                  file_str + "-albedos",
                  asset_loader::image_type::JPG);

            if (export_normal)
                cr::asset_loader::export_framebuffer(
                  *renderer->get()->current_normals(),
                  file_str + "-normals",
                  asset_loader::image_type::JPG);

            if (export_depth)
                cr::asset_loader::export_framebuffer(
                  *renderer->get()->current_depths(),
                  file_str + "-depth",
                  asset_loader::image_type::JPG);

            auto to_post = *data;
            if (denoise)
            {
                const auto denoised = cr::denoise(
                  renderer->get()->current_progress(),
                  renderer->get()->current_normals(),
                  renderer->get()->current_albedos(),
                  selected_type);

                if (post_process) to_post = denoised;

                cr::asset_loader::export_framebuffer(
                  denoised,
                  file_str + "-denoised",
                  selected_type);
            }

            if (post_process)
            {
                const auto processed = processor->get()->process(to_post);
                cr::asset_loader::export_framebuffer(
                  processed,
                  file_str + "-processed",
                  selected_type);
            }

            cr::logger::info("Finished exporting image in [{}s]", timer.time_since_start());
        }
    }

    inline void setting_camera(cr::renderer *renderer, cr::scene *scene)
    {
        static auto camera = std::optional<cr::camera>();

        if (!camera.has_value()) camera = *scene->registry()->camera();

        static const auto camera_modes =
          std::array<std::string, 2>({ "Perspective", "Orthographic" });

        static auto current_type = 0;

        if (ImGui::BeginCombo("Camera Mode", camera_modes[current_type].c_str()))
        {
            for (auto i = 0; i < camera_modes.size(); i++)
                if (ImGui::Button(camera_modes[i].c_str())) current_type = i;
            ImGui::EndCombo();
        }

        auto selected_type = cr::camera::mode::perspective;

        switch (current_type)
        {
        case 0:
        {
            selected_type = cr::camera::mode::perspective;
            ImGui::SliderFloat("FOV", &camera.value().fov, 5, 180);
            break;
        }
        case 1:
        {
            selected_type = cr::camera::mode::orthographic;
            ImGui::InputFloat("Scale", &camera.value().scale, 1, 10);
            break;
        }
        }

        camera->current_mode = selected_type;

        if (ImGui::Button("Update"))
        {
            renderer->update([scene, camera = camera]
                             { *scene->registry()->camera() = camera.value(); });
            camera.reset();
        }
    }

    inline void setting_materials(
      cr::renderer *                                                 renderer,
      cr::scene *                                                    scene,
      std::array<key_state, static_cast<size_t>(key_code::MAX_KEY)> &keys)
    {
        const auto models =
          scene->registry()->entities.view<std::string, cr::entity::model_materials>();

        static auto selected_entity  = uint32_t(0);
        bool        selected_changed = false;

        {
            for (const auto entity : models)
            {
                const auto &name = scene->registry()->entities.get<std::string>(entity);
                if (ImGui::Button(name.c_str()))
                {
                    if (selected_entity != entity) selected_changed = true;
                    selected_entity = entity;
                }
            }
        }

        if (selected_entity != 0)
        {
            const auto &registry_materials =
              scene->registry()->entities.get<cr::entity::model_materials>(selected_entity);

            ImGui::BeginChild("settings-materials-materials-list");

            static auto material_search_string = std::array<char, 65>();
            material_search_string[64]         = '\0';
            const auto changed                 = ImGui::InputTextWithHint(
              "Material name",
              "Max 64 chars",
              material_search_string.data(),
              64);

            static auto materials  = std::vector<cr::material>();
            static auto first_time = true;
            if (selected_changed || first_time)
            {
                selected_changed = false;
                materials.clear();
                materials = registry_materials.materials;
            }

            // This is a cool Imgui thing im going to make (search thing)
            static auto found_material_indices = std::vector<size_t>();

            // noooooo, std::vector<bool>
            static auto found_material_selected = std::vector<uint8_t>();
            static auto any_selection           = false;

            if (changed || first_time)
            {
                found_material_indices = cr::algorithm::find_string_matches<cr::material>(
                  std::string(material_search_string.data()),
                  materials,
                  [](const cr::material &material) { return material.info.name; });
                found_material_selected.resize(found_material_indices.size(), false);
            }

            any_selection = false;
            for (auto enabled : found_material_selected)
                if (enabled)
                {
                    any_selection = true;
                    break;
                }

            for (auto i = 0; i < found_material_indices.size(); i++)
            {
                const auto index    = i;
                auto &     material = materials[index];
                const auto selected = found_material_selected[i];

                ImGui::Separator();
                ImGui::Indent(4.f);
                ImGui::Text(
                  "%s",
                  ((found_material_selected[i] ? "* " : "") + material.info.name).c_str());

                if (!selected && any_selection)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6, 0.6, 0.6, 1.0));

                // Check if it was attempted to be selected
                if (
                  ImGui::IsItemHovered() && (keys[key_code::KEY_LEFT_CONTROL] == key_state::held || keys[key_code::KEY_LEFT_CONTROL] == key_state::repeat) && ImGui::IsMouseDoubleClicked(0))
                {

                    if (found_material_selected[i])
                        found_material_selected[i] = false;
                    else if (keys[key_code::KEY_LEFT_SHIFT] == key_state::held || keys[key_code::KEY_LEFT_SHIFT] == key_state::repeat)
                    {
                        auto closest_selection = std::int32_t(-1);

                        // iterate down
                        for (auto j = i; j >= 0; j--)
                            if (found_material_selected[j])
                            {
                                closest_selection = j;
                                break;
                            }

                        // iterate up
                        for (auto j = i; j < found_material_selected.size(); j++)
                            if (found_material_selected[j])
                            {
                                if (j - i < closest_selection || closest_selection == -1)
                                    closest_selection = j;
                                break;
                            }

                        if (closest_selection == -1)
                        {
                            // Dont auto select the other indices
                            found_material_selected[i] = true;
                        }
                        else
                        {
                            // Select the indices between the closest one
                            auto start = glm::min(closest_selection, i);
                            auto end   = glm::max(closest_selection, i);

                            for (auto j = start; j <= end; j++) found_material_selected[j] = true;
                        }
                    }
                    else
                        found_material_selected[i] = true;
                }
                ImGui::Indent(4.f);
                static const auto material_types =
                  std::array<std::string, 3>({ "Metal", "Smooth", "Glass" });
                auto current_type = material.info.shade_type == material::metal ? 0
                  : material.info.shade_type == material::smooth                ? 1
                                                                                : 2;

                auto update_type = false;
                if (ImGui::BeginCombo(
                      ("Type##" + material.info.name).c_str(),
                      material_types[current_type].c_str()))
                {
                    for (auto j = 0; j < material_types.size(); j++)
                        if (ImGui::Button(material_types[j].c_str()))
                        {
                            if (current_type != j) update_type = true;

                            current_type = j;
                        }
                    ImGui::EndCombo();
                }

                switch (current_type)
                {
                case 0: material.info.shade_type = material::metal; break;
                case 1: material.info.shade_type = material::smooth; break;
                case 2: material.info.shade_type = material::glass; break;
                }

                if (update_type && any_selection && selected)
                    for (auto j = 0; j < found_material_indices.size(); j++)
                        if (found_material_selected[j])
                            materials[found_material_indices[j]].info.shade_type =
                              material.info.shade_type;

                ImGui::Separator();

                switch (material.info.shade_type)
                {
                case material::metal:
                    //                    ImGui::SliderFloat(
                    //                      ("Roughness##" + material.info.name).c_str(),
                    //                      &material.info.roughness,
                    //                      0,
                    //                      1);
                    if (
                      widgets::slider_float_input(
                        "Reflectiveness##" + material.info.name,
                        material.info.reflectiveness,
                        0,
                        1) &&
                      any_selection && selected)
                    {
                        for (auto j = 0; j < found_material_indices.size(); j++)
                            if (found_material_selected[j])
                                materials[found_material_indices[j]].info.reflectiveness =
                                  material.info.reflectiveness;
                    }
                    break;

                case material::smooth: break;

                case material::glass:
                    if (
                      widgets::slider_float_input(
                        "IOR##" + material.info.name,
                        material.info.ior,
                        1,
                        2) &&
                      any_selection && selected)
                    {
                        for (auto j = 0; j < found_material_indices.size(); j++)
                            if (found_material_selected[j])
                                materials[found_material_indices[j]].info.ior = material.info.ior;
                    }
                    break;
                }

                if (
                  widgets::slider_float_input(
                    "Emission##" + material.info.name,
                    material.info.emission,
                    0,
                    50) &&
                  any_selection && selected)
                {
                    for (auto j = 0; j < found_material_indices.size(); j++)
                        if (found_material_selected[j])
                            materials[found_material_indices[j]].info.emission =
                              material.info.emission;
                }

                if (!material.info.tex.has_value())
                    if (
                      ImGui::ColorEdit3(
                        ("Colour##" + material.info.name).c_str(),
                        glm::value_ptr(material.info.colour)) &&
                      any_selection && selected)
                    {
                        for (auto j = 0; j < found_material_indices.size(); j++)
                            if (found_material_selected[j])
                                materials[found_material_indices[j]].info.colour =
                                  material.info.colour;
                    }


                if (!selected && any_selection)
                    ImGui::PopStyleColor();
                ImGui::Unindent(8.f);
            }

            if (ImGui::Button("Update Materials"))
            {
                renderer->update(
                  [materials = materials, scene, selected = selected_entity] {
                      scene->registry()
                        ->entities.get<cr::entity::model_materials>(selected)
                        .materials = materials;
                  });
            }

            ImGui::EndChild();
            first_time = false;
        }
    }

    inline void setting_asset_loader(
      std::unique_ptr<cr::renderer> *renderer,
      std::unique_ptr<cr::scene> *   scene,
      bool                           in_draft_mode)
    {
        static std::string current_directory;
        static std::string current_model;
        bool               throw_away = false;
        ImGui::Indent(4.f);

        ImGui::Text("Model Loader");
        ImGui::Indent(4.f);
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
            cr::logger::info("Starting to load model [{}]", current_model);
            auto timer = cr::timer();
            // Load model in
            const auto model_data = cr::asset_loader::load_model(current_model, current_directory);

            if (!in_draft_mode)
                renderer->get()->update([&scene, &model_data]
                                        { scene->get()->add_model(model_data); });
            else
                scene->get()->add_model(model_data);
            cr::logger::info("Finished loading model in [{}s]", timer.time_since_start());

            auto texture_count = 0;
            for (const auto &material : model_data.materials)
                if (material.info.tex.has_value()) texture_count++;

            cr::logger::info(
              "-- Model Stats\n\tVertices: [{}]\n\tTriangles: [{}]\n\tMaterials: [{}]\n\tTextures: "
              "[{}]",
              model_data.vertices.size(),
              model_data.vertex_indices.size() / 3,
              model_data.materials.size(),
              texture_count);
        }

        ImGui::Unindent(4.f);
        ImGui::Separator();
        ImGui::NewLine();
        ImGui::Text("Skybox Loader");
        ImGui::Indent(4.f);

        static std::filesystem::path current_skybox;
        if (ImGui::BeginCombo("Select Skybox", current_skybox.string().c_str()))
        {
            for (const auto &entry : std::filesystem::directory_iterator("./assets/skybox"))
            {
                if (entry.is_directory()) continue;

                if (ImGui::Selectable(entry.path().filename().string().c_str(), &throw_away))
                {
                    current_skybox = entry.path();
                    break;
                }
            }
            ImGui::EndCombo();
        }

        if (!current_skybox.empty() && ImGui::Button("Load Skybox"))
        {
            auto timer = cr::timer();
            // Load skybox in
            cr::logger::info("Started to load skybox [{}]", current_skybox.stem().string());
            renderer->get()->update(
              [&scene, current_skybox = current_skybox, &timer]
              {
                  auto image  = cr::asset_loader::load_picture(current_skybox.string());
                  auto skybox = cr::image(image.colour, image.res.x, image.res.y);

                  scene->get()->set_skybox(std::move(skybox));

                  cr::logger::info("Finished loading skybox in [{}s]", timer.time_since_start());
                  cr::logger::info(
                    "-- Skybox Stats\n\tResolution:\n\t\tX: [{}]\n\t\tY: [{}]",
                    image.res.x,
                    image.res.y);
              });
        }

        static auto rotation = glm::vec2();
        ImGui::DragFloat2("Rotation", glm::value_ptr(rotation), 0.f, 1.f);

        ImGui::SameLine();
        if (ImGui::Button("Update"))
            renderer->get()->update([&scene]()
                                    { scene->get()->set_skybox_rotation(rotation / 360.f); });

        ImGui::Unindent(8.f);
    }

    inline void setting_stats(cr::renderer *renderer)
    {
        ImGui::Indent(4.f);

        const auto stats = renderer->current_stats();

        ImGui::Text("%s", fmt::format("Rays per second: [{}]", stats.rays_per_second).c_str());
        ImGui::Text(
          "%s",
          fmt::format("Samples per second: [{}]", stats.samples_per_second).c_str());
        ImGui::Text("%s", fmt::format("Total Rays Fired: [{}]", stats.total_rays).c_str());
        ImGui::Text("%s", fmt::format("Running Time: [{}]", stats.running_time).c_str());

        ImGui::Unindent(4.f);
    }

    inline std::optional<cr::ImGuiThemes::theme> new_theme;
    inline void                                  setting_style()
    {
        ImGui::Separator();
        ImGui::Indent(4.0f);
        ImGui::Text("Custom Theme (?)");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Don't like the default colours? Spice it up a bit!");

        static std::string current_directory;
        static std::string current_font;
        bool               throw_away = false;
        ImGui::Indent(4.0f);

        ImGui::BeginChild("style-child-region", { 0, ImGui::GetContentRegionAvail().y / 5 });

        static const auto styles =
          std::array<std::string, 4>({ "Red", "Corporate Grey", "Cherry", "Dark Charcoal" });

        for (auto i = 0; i < styles.size(); i++)
            if (ImGui::Button(styles[i].c_str()))
                if (i == 0)
                    new_theme = cr::ImGuiThemes::theme::RED;
                else if (i == 1)
                    new_theme = cr::ImGuiThemes::theme::CORPORATE_GREY;
                else if (i == 2)
                    new_theme = cr::ImGuiThemes::theme::CHERRY;
                else if (i == 3)
                    new_theme = cr::ImGuiThemes::theme::DARK_CHARCOAL;
                else if (i == 4)
                    new_theme = cr::ImGuiThemes::theme::VISUAL_STUDIO;
                else if (i == 5)
                    new_theme = cr::ImGuiThemes::theme::GREEN;

        ImGui::EndChild();

        ImGui::Unindent(8.0f);
    }

    inline void setting_instances(cr::renderer *renderer, cr::scene *scene)
    {
        ImGui::Indent(4.0f);

        const auto models = scene->registry()->entities.view<std::string, cr::entity::instances>();

        static auto selected_entity  = uint32_t(0);
        bool        selected_changed = false;

        {
            for (const auto entity : models)
            {
                const auto &name = scene->registry()->entities.get<std::string>(entity);
                if (ImGui::Button(name.c_str()))
                {
                    if (selected_entity != entity) selected_changed = true;
                    selected_entity = entity;
                }
            }
        }

        if (selected_entity != 0)
        {
            const auto &registry_instances =
              scene->registry()->entities.get<cr::entity::instances>(selected_entity);

            static auto transforms = std::vector<glm::mat4>();
            static auto first_time = true;

            if (selected_changed || first_time)
            {
                first_time = false;
                transforms = registry_instances.transforms;
            }

            ImGui::BeginChild("settings-instances-instances-list");

            ImGui::Text("Instances");
            ImGui::SameLine();
            if (ImGui::Button("+")) transforms.push_back(glm::mat4(1));

            ImGui::Indent(4.f);

            static auto to_remove = std::vector<uint32_t>();

            for (auto i = 0; i < transforms.size(); i++)
            {
                const auto matrix      = transforms[i];
                auto       skew        = glm::vec3();
                auto       perspective = glm::vec4();
                auto       rotation    = glm::quat();

                // Ones I use...
                auto scale       = glm::vec3();
                auto translation = glm::vec3();

                glm::decompose(matrix, scale, rotation, translation, skew, perspective);

                ImGui::Text("%s", fmt::format("Translation #{}", i).c_str());
                ImGui::SameLine();
                if (ImGui::Button("-")) to_remove.push_back(i);

                ImGui::Indent(4.0f);
                ImGui::InputFloat3(fmt::format("Scale##{}", i).c_str(), glm::value_ptr(scale));
                ImGui::InputFloat3(
                  fmt::format("Translation##{}", i).c_str(),
                  glm::value_ptr(translation));
                ImGui::Unindent(4.0f);

                auto out_matrix = glm::mat4(1);
                out_matrix      = glm::translate(out_matrix, translation);
                out_matrix      = glm::scale(out_matrix, scale);

                transforms[i] = out_matrix;
            }

            for (const auto remove : to_remove) transforms.erase(transforms.begin() + remove);

            if (ImGui::Button("Update"))
            {
                renderer->update(
                  [scene, transforms = transforms, selected_entity = selected_entity]()
                  {
                      scene->registry()
                        ->entities.get<cr::entity::instances>(selected_entity)
                        .transforms = transforms;
                  });
            }
            ImGui::Unindent(4.0f);
            ImGui::EndChild();
            to_remove.clear();
        }

        ImGui::Unindent(4.0f);
    }

    inline void setting_post_process(cr::post_processor &processor)
    {
        static auto bloom      = post_processor::bloom_settings();
        static auto gray_scale = post_processor::gray_scale_settings();
        static auto tonemap    = post_processor::tonemapping_settings();
        ImGui::Checkbox("Use Bloom", &bloom.enabled);
        if (bloom.enabled)
        {
            ImGui::Indent(4.f);
            ImGui::InputFloat("Threshold", &bloom.threshold);
            ImGui::InputFloat("Bloom Strength", &bloom.strength);
            ImGui::Unindent(4.f);
        }

        ImGui::Checkbox("Use Gray Scale", &gray_scale.enabled);

        ImGui::Checkbox("Use Tonemapping", &tonemap.enabled);
        if (tonemap.enabled)
        {
            ImGui::Indent(4.f);
            ImGui::InputFloat("Exposure", &tonemap.exposure);
            static const auto tonemapping_operators = std::array<std::string, 4>(
              { "Linear", "Reinhard", "Jim and Richard", "Uncharted 2" });
            static auto selected_window = 0;

            if (selected_window != 2)
                ImGui::InputFloat("Gamma Correction", &tonemap.gamma_correction);

            if (ImGui::BeginCombo(
                  "Tonemapping Operator",
                  tonemapping_operators[selected_window].c_str()))
            {
                for (auto i = 0; i < tonemapping_operators.size(); i++)
                    if (ImGui::Button(tonemapping_operators[i].c_str())) selected_window = i;
                ImGui::EndCombo();
            }
            tonemap.type = selected_window;
            ImGui::Unindent(4.f);
        }

        if (ImGui::Button("Update"))
        {
            processor.submit_bloom_settings(bloom);
            processor.submit_gray_scale_settings(gray_scale);
            processor.submit_tonemapping_settings(tonemap);
        }
    }

    inline void settings(
      std::unique_ptr<cr::renderer> *                                renderer,
      std::unique_ptr<cr::draft_renderer> *                          draft_renderer,
      std::unique_ptr<cr::scene> *                                   scene,
      std::unique_ptr<cr::thread_pool> *                             pool,
      std::unique_ptr<cr::post_processor> *                          post_processor,
      std::array<key_state, static_cast<size_t>(key_code::MAX_KEY)> &keys,
      bool                                                           draft_mode,
      glm::vec2 &speed_multipliers)
    {
        ImGui::Begin("Misc");

        // List all of the different settings
        static const auto window_settings = std::array<std::string, 9>({ "Render",
                                                                         "Export",
                                                                         "Materials",
                                                                         "Asset Loader",
                                                                         "Stats",
                                                                         "Style",
                                                                         "Camera",
                                                                         "Instances",
                                                                         "Post Processing" });

        static auto selected_window = 0;

        for (auto i = 0; i < window_settings.size(); i++)
        {
            if (i % 4 != 0) ImGui::SameLine();
            if (ImGui::Button(window_settings[i].c_str())) selected_window = i;
        }

        ImGui::Separator();

        ImGui::BeginChild(window_settings[selected_window].c_str(), ImVec2(), true);

        ImGui::Text("%s", window_settings[selected_window].c_str());

        ImGui::Separator();

        switch (selected_window)
        {
        case 0: setting_render(renderer->get(), draft_renderer->get(), scene->get(), *pool, speed_multipliers); break;
        case 1: setting_export(renderer, post_processor); break;
        case 2: setting_materials(renderer->get(), scene->get(), keys); break;
        case 3: setting_asset_loader(renderer, scene, draft_mode); break;
        case 4: setting_stats(renderer->get()); break;
        case 5: setting_style(); break;
        case 6: setting_camera(renderer->get(), scene->get()); break;
        case 7: setting_instances(renderer->get(), scene->get()); break;
        case 8: setting_post_process(**post_processor); break;
        }

        ImGui::EndChild();

        ImGui::End();
    }

    inline void console(const std::vector<std::string> &lines = {})
    {
        ImGui::Begin("Console");

        ImGui::Text("Console Output");
        ImGui::Separator();

        ImGui::BeginChild("Console-Text-Region");

        static auto lines_to_display = std::vector<std::string>();
        lines_to_display.reserve(lines_to_display.size() + lines.size());
        for (const auto &string : lines) lines_to_display.push_back(string);

        for (const auto &line : lines_to_display) ImGui::Text("%s", line.c_str());

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();

        ImGui::End();
    }

    //    inline void export_ui

}    // namespace cr::ui
