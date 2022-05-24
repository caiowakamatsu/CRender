//
// Created by Caio W on 11/2/22.
//

#include "sphere_scene.hpp"

#include <material/material.hpp>

std::optional<cr::intersection>
cr::sphere_scene::intersect(const cr::ray &ray) {
  //	static auto diffuse = std::make_unique<cr::diffuse>(glm::vec3(0.0f),
  //glm::vec3(1.0f, 1.0f, 1.0f));

  const auto origin = glm::vec3(0, 0, 0);
  const auto r2 = 8.0f;

  const auto oc = ray.origin - origin;

  const auto b = 2.0f * glm::dot(oc, ray.direction);
  const auto c = glm::dot(oc, oc) - r2 * r2;
  const auto disc = b * b - 4.0f * c;

  if (disc < 0.0f) {
    return std::nullopt;
  } else {
    const auto distance = -b - glm::sqrt(disc) / 2.0f;
    const auto point = ray.at(distance);
    //		return cr::intersection {
    //						.distance = distance,
    //						.material = diffuse.get(),
    //						.normal = glm::normalize(point -
    //origin)
    //		};
    return {};
  }

  return std::optional<cr::intersection>();
}
