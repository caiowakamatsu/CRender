//
// Created by Caio W on 11/2/22.
//

#ifndef CREBON_SPHERE_SCENE_HPP
#define CREBON_SPHERE_SCENE_HPP

#include <optional>
#include <scene/ray.hpp>
#include <scene/scene.hpp>

namespace cr {
class sphere_scene {
public:
  [[nodiscard]] std::optional<cr::intersection> intersect(const cr::ray &ray);
};
} // namespace cr

#endif // CREBON_SPHERE_SCENE_HPP
