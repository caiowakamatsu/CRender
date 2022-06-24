//
// Created by Caio W on 11/6/22.
//

#ifndef CREBON_POST_PROCESSING_HPP
#define CREBON_POST_PROCESSING_HPP

namespace cr::component {

class post_processing {
public:
  struct Options {
    bool gamma_correct = true;
    float exposure = 1.0f;
    bool denoise = false;
  };

  struct DisplayContents {
  };

  struct Component {
    [[nodiscard]] post_processing::Options display(DisplayContents contents) const;
  };
};

} // namespace cr::component

#endif // CREBON_POST_PROCESSING_HPP
