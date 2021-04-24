#pragma once

#include <string>

#include <imgui/imgui.h>

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
              0.175,
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
}    // namespace cr::ui
