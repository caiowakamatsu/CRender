//
// Created by Caio W on 12/2/22.
//

#include "preview.hpp"

#include <util/random.hpp>

#include <imgui.h>

#include <glad/glad.h>

cr::component::preview::Options
cr::component::preview::Component::display(DisplayContents contents) const {
  ImGui::Begin("Preview");

  static auto in_post = false;

  ImGui::BeginTabBar("display_preview_tab");
  if (ImGui::TabItemButton("Raw Output")) {
    in_post = false;
  }
  if (ImGui::TabItemButton("Post Processed")) {
    in_post = true;
  }
  ImGui::EndTabBar();

  static auto texture = GLuint();
  static auto first = true;

  auto window_size = ImGui::GetContentRegionAvail();
  if (window_size.x < 0 || window_size.y < 0) {
    ImGui::End();
    return {}; // During initialisation sometimes the window size is negative
  }

  if (first) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    first = false;
  }

  const auto win_size = glm::vec2(window_size.x, window_size.y);
  const auto w = contents.frame->height() * win_size.x;
  const auto h = contents.frame->width() * win_size.y;

  const auto [n, d] = std::minmax(w, h);
  const auto f = n / d;
  const auto x = w < h;

  auto theoretical_size = win_size;
  theoretical_size[static_cast<int>(x)] *= f;

  const auto start = (win_size - theoretical_size) * 0.5f;
  const auto end = start + theoretical_size;

  const auto post_processing = in_post;
  const auto process_pixel =
      [post_processing, &contents]() -> std::function<glm::vec4(glm::vec4)> {
    if (post_processing && contents.gamma_correct) {
      return [&contents](glm::vec4 pixel) -> glm::vec4 {
        return glm::pow(pixel, glm::vec4(1.0f / 2.2f));
      };
    } else {
      return [](glm::vec4 pixel) -> glm::vec4 { return pixel; };
    }
  }();

  auto data = std::vector<glm::vec3>(window_size.x * window_size.y);
  {
    for (size_t y = start.y; y < end.y; y++) {
      for (size_t x = start.x; x < end.x; x++) {
        data[x + y * window_size.x] = process_pixel(contents.frame->get_uv(
            (x - start.x) / theoretical_size.x,
            1.0f - ((y - start.y) / theoretical_size.y)));
      }
    }
  }

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, window_size.x, window_size.y, 0,
               GL_RGB, GL_FLOAT, data.data());

  if (!first) {
    ImGui::Image((void *)texture, window_size);
  }

  ImGui::End();
  return {};
}
