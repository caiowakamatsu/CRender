//
// Created by Caio W on 12/2/22.
//

#ifndef CREBON_CONSOLE_HPP
#define CREBON_CONSOLE_HPP

#include <string>
#include <vector>

#include <util/logger.hpp>

namespace cr::component {
class console {
public:
  struct Options {};

  struct DisplayContents {
    std::vector<std::pair<cr::logger::level, std::string>> *lines;
  };

  struct Component {
    [[nodiscard]] console::Options display(DisplayContents contents) const;
  };
};
} // namespace cr::component

#endif // CREBON_CONSOLE_HPP
