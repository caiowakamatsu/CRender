//
// Created by howto on 29/5/2022.
//

#ifndef CREBON_STATS_HPP
#define CREBON_STATS_HPP

namespace cr::component {
class stats {
public:
  struct Options {};

  struct DisplayContents {
    int samples_per_second;
    int total_samples;
    int rays_per_second;
    int total_instances;
    double total_render_time;
  };

  struct Component {
    [[nodiscard]] stats::Options display(DisplayContents contents) const;
  };
};
} // namespace cr::component

#endif // CREBON_STATS_HPP
