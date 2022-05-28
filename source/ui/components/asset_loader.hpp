//
// Created by howto on 24/5/2022.
//

#ifndef CREBON_ASSET_LOADER_HPP
#define CREBON_ASSET_LOADER_HPP

#include <filesystem>
#include <string>
#include <string_view>

namespace cr::component {
struct asset_loader {

  struct Options {
    std::filesystem::path load;
  };
  struct DisplayContents {};
  struct Component {
    [[nodiscard]] asset_loader::Options display(DisplayContents contents) const;
  };
};
} // namespace cr::component

#endif // CREBON_ASSET_LOADER_HPP
