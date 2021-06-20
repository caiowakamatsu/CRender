#pragma once

#include <string>

#include <imgui/imgui.h>
#include <render/renderer.h>
#include <render/timer.h>
#include <render/draft/draft_renderer.h>
#include <util/asset_loader.h>

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
      const std::string &left_bottom,
      const std::string &left_middle,
      const std::string &left_top,
      const std::string &middle,
      const std::string &right_bottom,
      const std::string &right_middle,
      const std::string &right_top)
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

            auto dock_id_right_bottom = ImGui::DockBuilderSplitNode(
              dockspace_id,
              ImGuiDir_Right,
              0.15f,
              nullptr,
              &dockspace_id);

            auto dock_id_right_top = ImGui::DockBuilderSplitNode(
              dock_id_right_bottom,
              ImGuiDir_Up,
              .175,
              nullptr,
              &dock_id_right_bottom);

            auto dock_id_right_middle = ImGui::DockBuilderSplitNode(
              dock_id_right_bottom,
              ImGuiDir_Up,
              0.85f,
              nullptr,
              &dock_id_right_bottom);

            auto dock_id_left_bottom = ImGui::DockBuilderSplitNode(
              dockspace_id,
              ImGuiDir_Left,
              0.3f,
              nullptr,
              &dockspace_id);
            auto dock_id_left_top = ImGui::DockBuilderSplitNode(
              dock_id_left_bottom,
              ImGuiDir_Up,
              0.12f,
              nullptr,
              &dock_id_left_bottom);
            auto dock_id_left_middle = ImGui::DockBuilderSplitNode(
              dock_id_left_bottom,
              ImGuiDir_Up,
              0.12f,
              nullptr,
              &dock_id_left_bottom);

            auto dock_id_middle = dockspace_id;

            ImGui::DockBuilderDockWindow(left_bottom.c_str(), dock_id_left_bottom);
            ImGui::DockBuilderDockWindow(left_middle.c_str(), dock_id_left_middle);
            ImGui::DockBuilderDockWindow(left_top.c_str(), dock_id_left_top);

            ImGui::DockBuilderDockWindow(right_bottom.c_str(), dock_id_right_bottom);
            ImGui::DockBuilderDockWindow(right_middle.c_str(), dock_id_right_middle);
            ImGui::DockBuilderDockWindow(right_top.c_str(), dock_id_right_top);

            ImGui::DockBuilderDockWindow(middle.c_str(), dock_id_middle);
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    inline void root_node(init_ctx ui_ctx)
    {
        ImGui::Begin("DockSpace", nullptr, ui_ctx.window_flags);
        ImGui::PopStyleVar(3);

        // Setup the dock
        cr::ui::init_dock(
          ui_ctx,
          "Scene Objects",
          "Skybox",
          "Model Loader",
          "Scene Preview",
          "Export",
          "Property Editor",
          "Render");

        ImGui::End();
    }

    inline void render_quality(cr::renderer* renderer)
    {
        ImGui::Begin("Render Quality");

        static auto resolution = glm::ivec2();
        static auto bounces    = int(5);

        if (resolution.x == 0)
            resolution.x = renderer->current_resolution().x;

        if (resolution.y == 0)
            resolution.y = renderer->current_resolution().y;

        ImGui::InputInt2("Resolution", glm::value_ptr(resolution));
        ImGui::InputInt("Max Bounces", &bounces);

        if (ImGui::Button("Update"))
        {
            renderer->update([renderer]() {
              renderer->set_max_bounces(bounces);
              renderer->set_resolution(resolution.x, resolution.y);
            });
        }

        ImGui::End();
    }

//    inline void export_ui

}    // namespace cr::ui
