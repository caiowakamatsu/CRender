//
// Created by howto on 24/5/2022.
//

#ifndef CREBON_EXPORT_HPP
#define CREBON_EXPORT_HPP

namespace cr::component {

class image_export {
public:
  struct Options {};
  struct DisplayContents {};

  struct Component {
    [[nodiscard]] image_export::Options display(DisplayContents contents) const;
  };
};

} // namespace cr::component

#endif // CREBON_EXPORT_HPP
