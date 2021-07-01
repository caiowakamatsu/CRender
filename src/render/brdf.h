#pragma once

#include <render/ray.h>
#include <util/sampling.h>

#include <glm/glm.hpp>

namespace cr::brdf
{
    [[nodiscard]] inline glm::vec3 ggx(
      const cr::ray &  incoming,
      const cr::ray &  outgoing,
      const glm::vec3 &normal,
      const float      roughness,
      const float ior)
    {
        const auto h = glm::normalize(outgoing.direction + -incoming.direction);
        const auto n_h = glm::dot(normal, h);

        const auto n_l     = glm::max(0.0f, glm::dot(normal, outgoing.direction));
        const auto l_h     = glm::dot(outgoing.direction, h);
        const auto n_dot_v = glm::dot(normal, -incoming.direction);

        const auto ggx_d = cr::sampling::cook_torrence::specular_d(n_h, roughness);
        const auto ggx_g = cr::sampling::cook_torrence::specular_g(n_l, n_dot_v, roughness);
        const auto ggx_f = cr::sampling::cook_torrence::specular_f(ior, l_h);

        return (ggx_d * ggx_g * ggx_f / glm::max(0.000001f, (4 * n_dot_v)));
    }
}    // namespace cr::brdf
