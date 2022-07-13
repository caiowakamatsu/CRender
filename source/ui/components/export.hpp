//
// Created by howto on 24/5/2022.
// Modified by Yohwllo
//

#ifndef CREBON_EXPORT_HPP
#define CREBON_EXPORT_HPP

#include <util/atomic_image.hpp>

#include <string>

namespace cr::component {

class image_export {
public:
  struct Options {
    std::string scene_name = std::string(512, '\0');
    int image_type = 0;
  };
  struct DisplayContents {
    cr::atomic_image *image;
  };

  struct Component {
    [[nodiscard]] image_export::Options display(DisplayContents contents) const;
  };
};

} // namespace cr::component

#endif // CREBON_EXPORT_HPP
