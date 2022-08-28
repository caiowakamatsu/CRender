//
// Created by Caio W on 12/2/22.
//

#ifndef CREBON_SETTINGS_HPP
#define CREBON_SETTINGS_HPP

#include <optional>

#include <ui/components/asset_loader.hpp>
#include <ui/components/export.hpp>
#include <ui/components/render_target.h>
#include <ui/components/skybox.hpp>
#include <ui/components/post_processing.hpp>

#include <ui/components/component.hpp>

namespace cr::component
{

  // Todo: at some point make this fully configurable / reusable
  class settings
  {
  public:
    struct Options
    {
      std::optional<component::render_target::Options> render_target;
      std::optional<component::image_export::Options> image_export;
      std::optional<component::asset_loader::Options> asset_loader;
      std::optional<component::skybox::Options> skybox;
      std::optional<component::post_processing::Options> post_processing;
    };
    struct DisplayContents
    {
    };

    struct Component
    {
    private:
      enum class sub_settings
      {
        render_target,
        image_export,
        asset_loader,
        skybox,
        post_processing,
      };

      struct
      {
        component::interface<component::render_target> render_target;
        component::interface<component::image_export> image_export;
        component::interface<component::asset_loader> asset_loader;
        component::interface<component::skybox> skybox;
        component::interface<component::post_processing> post_processing;
      } _sub_settings{};

      std::optional<sub_settings> _selected;

    public:
      [[nodiscard]] settings::Options display(DisplayContents contents);
    };
  };
} // namespace cr::component

#endif // CREBON_SETTINGS_HPP
