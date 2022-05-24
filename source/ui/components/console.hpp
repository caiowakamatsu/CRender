//
// Created by Caio W on 12/2/22.
//

#ifndef CREBON_CONSOLE_HPP
#define CREBON_CONSOLE_HPP

#include <string>
#include <vector>

namespace cr::component {
class console {
public:
  struct Options {};

  struct DisplayContents {
    std::vector<std::string> *lines;
  };

  struct Component {
    [[nodiscard]] console::Options display(DisplayContents contents) const;
  };
};
} // namespace cr::component

#endif // CREBON_CONSOLE_HPP
