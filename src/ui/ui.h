#pragma once

#include <string>

#include <imgui/imgui.h>
#include <render/renderer.h>
#include <render/timer.h>
#include <render/draft/draft_renderer.h>
#include <util/asset_loader.h>
#include <stb/stbi_image_write.h>
#include <stb/stb_image.h>
#include "display.h"

namespace cr::ui
{
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
      GLuint              target_texture,
      GLuint              scene_texture,
      GLuint              compute_program,
      bool in_draft_mode)
    {
        ImGui::Begin("Scene Preview");
        auto window_size = ImGui::GetContentRegionAvail();

        if (in_draft_mode)
        {
            draft_renderer->render();
            ImGui::Image((void *) draft_renderer->rendered_texture(), window_size);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, target_texture);
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
            glBindImageTexture(0, target_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

            const auto current_progress = renderer->current_progress();
            // Upload rendered scene to GPU
            glBindTexture(GL_TEXTURE_2D, scene_texture);
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
                auto io = ImGui::GetIO();
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
                  glGetUniformLocation(compute_program, "translation"),
                  1,
                  glm::value_ptr(current_translation));

                glUniform1f(glGetUniformLocation(compute_program, "zoom"), current_zoom);
            }

            {
                glUseProgram(compute_program);

                glDispatchCompute(
                  static_cast<int>(window_size.x),
                  static_cast<int>(window_size.y),
                  1);

                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
            ImGui::Image((void *) scene_texture, window_size);
        }
        ImGui::End();
    }

    inline void setting_render(cr::renderer *renderer, std::unique_ptr<cr::thread_pool> &pool)
    {
        static auto resolution   = glm::ivec2();
        static auto bounces      = int(5);
        static auto thread_count = static_cast<int>(std::thread::hardware_concurrency());

        if (resolution.x == 0) resolution.x = renderer->current_resolution().x;

        if (resolution.y == 0) resolution.y = renderer->current_resolution().y;

        ImGui::InputInt2("Resolution", glm::value_ptr(resolution));
        ImGui::InputInt("Max Bounces", &bounces);
        ImGui::InputInt("Thread Count", &thread_count);

        if (ImGui::Button("Update"))
        {
            renderer->update([renderer, &pool]() {
                renderer->set_max_bounces(bounces);
                renderer->set_resolution(resolution.x, resolution.y);
                pool = std::make_unique<cr::thread_pool>(thread_count);
            });
        }

        ImGui::Text(
          "%s",
          fmt::format("Current sample count: [{}]", renderer->current_sample_count()).c_str());
    }

    inline void setting_export(std::unique_ptr<cr::renderer> *renderer)
    {
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

            auto char_data = std::vector<uint8_t>(data->width() * data->height() * 4);
            for (auto i = 0; i < char_data.size(); i++) char_data[i] = data->data()[i] * 255.f;

            stbi_write_jpg(
              directory.c_str(),
              data->width(),
              data->height(),
              4,
              char_data.data(),
              100);
        }
    }

    inline void setting_materials() { ImGui::Text("This is currently being worked on"); }

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
            // Load model in
            const auto model_data = cr::asset_loader::load_model(current_model, current_directory);

            if (!in_draft_mode)
                renderer->get()->update(
                  [&scene, &model_data] { scene->get()->add_model(model_data); });
            else
                scene->get()->add_model(model_data);
        }

        ImGui::Unindent(4.f);
        ImGui::Separator();
        ImGui::NewLine();
        ImGui::Text("Skybox Loader");
        ImGui::Indent(4.f);

        static std::string current_skybox;
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

        static auto rotation = glm::vec2();
        ImGui::DragFloat2("Rotation", glm::value_ptr(rotation), 0.f, 1.f);

        ImGui::SameLine();
        if (ImGui::Button("Update"))
            renderer->get()->update([&scene](){
              scene->get()->set_skybox_rotation(rotation);
            });

        ImGui::Unindent(8.f);
    }

    inline void setting_stats() { ImGui::Text("This is still under construction"); }

    static auto new_font = std::string();

    inline void setting_style()
    {
        ImGui::Separator();
        ImGui::Indent(4.0f);
        ImGui::Text("Font Loader");

        static std::string current_directory;
        static std::string current_font;
        bool throw_away = false;
        ImGui::Indent(4.0f);
        if (ImGui::BeginCombo("Select Font", current_directory.c_str()))
        {
            for (const auto &entry : std::filesystem::directory_iterator("./assets/app/fonts"))
            {
                if (entry.is_directory()) continue;

                const auto font_path = cr::asset_loader::valid_font(entry);
                if (!font_path.has_value()) continue;

                // Go through each file in the directory
                if (ImGui::Selectable(entry.path().filename().string().c_str(), &throw_away))
                {
                    current_directory = entry.path().string();
                    current_font     = font_path.value();
                    break;
                }
            }
            ImGui::EndCombo();
        }

        if (current_font != std::filesystem::path() && ImGui::Button("Load Font"))
        {
            auto io = ImGui::GetIO();
            new_font = current_font;
        }
        ImGui::Unindent(8.0f);

    }

    inline void settings(
      std::unique_ptr<cr::renderer> *   renderer,
      std::unique_ptr<cr::scene> *      scene,
      std::unique_ptr<cr::thread_pool> *pool,
      bool                              draft_mode)
    {
        ImGui::Begin("Misc");

        // List all of the different settings
        static const auto window_settings =
          std::array<std::string, 6>({ "Render", "Export", "Materials", "Asset Loader", "Stats", "Settyle"});

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
        case 0: setting_render(renderer->get(), *pool); break;
        case 1: setting_export(renderer); break;
        case 2: setting_materials(); break;
        case 3: setting_asset_loader(renderer, scene, draft_mode); break;
        case 4: setting_stats(); break;
        case 5: setting_style(); break;
        }

        ImGui::EndChild();

        ImGui::End();
    }

    inline void console(const std::vector<std::string> &lines = {})
    {
        ImGui::Begin("Console");

        ImGui::Text("Console Output - Useful for nerds");
        ImGui::Separator();

        ImGui::BeginChild("Console-Text-Region");

        static auto lines_to_display = std::vector<std::string>();
        lines_to_display.reserve(lines_to_display.size() + lines.size());
        for (const auto &string : lines)
            lines_to_display.push_back(string);

        for (const auto &line : lines_to_display)
            ImGui::Text("%s", line.c_str());

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();

        ImGui::End();
    }

    //    inline void export_ui

}    // namespace cr::ui
