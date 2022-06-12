//
// Created by Caio W on 12/2/22.
//

#ifndef CREBON_PREVIEW_HPP
#define CREBON_PREVIEW_HPP

#include <util/image.hpp>
#include <ui/components/post_processing.hpp> // ideally this shouldn't exist but...

#include <mutex>

namespace cr::component {
struct preview {
public:
  struct Options {};
  struct DisplayContents {
    cr::atomic_image *frame {};
    cr::component::post_processing::Options post;
  };

  struct Component {
    [[nodiscard]] preview::Options display(DisplayContents contents) const;
  };
};
} // namespace cr::component

#endif // CREBON_PREVIEW_HPP
