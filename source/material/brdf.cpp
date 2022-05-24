//
// Created by Caio W on 14/3/22.
//

#include "brdf.hpp"
#include "glm/ext/quaternion_common.hpp"

#include <glm/common.hpp>

glm::vec3 cr::brdf::gltf_brdf(const glm::vec3 &V, const glm::vec3 &L,
                              const glm::vec3 &N, const glm::vec3 &H,
                              const glm::vec3 &base_colour,
                              const float &roughness,
                              const float &metalness) noexcept {
  const auto black = glm::vec3(0.0f);

  const auto c_diff = glm::mix(base_colour, black, metalness);
  const auto f0 = glm::mix(glm::vec3(0.04f), base_colour, metalness);

  const auto F =
      f0 + (1.0f - f0) * glm::pow(1.0f - glm::abs(glm::dot(V, H)), 5.0f);

  const auto f_diffuse = (1.0f - F) * glm::one_over_pi<float>() * c_diff;
  const auto f_specular = F * ggx_microfacet(N, H, roughness);

  return f_diffuse + f_specular;
}

float cr::brdf::specular_brdf(const glm::vec3 &N, const glm::vec3 &L,
                              const glm::vec3 &V, const glm::vec3 &H, float a) {
  const auto d = brdf::ggx_microfacet(N, H, a);
  const auto v = brdf::visibility(H, V, L, N, a);
  return d * v;
}

float cr::brdf::ggx_microfacet(const glm::vec3 &N, const glm::vec3 &H,
                               float a) {
  const auto a2 = a * a;

  const auto N_H = glm::dot(N, H);
  const auto N_H_above_zero = N_H > 0.0f;

  const auto x = (N_H * N_H) * (a2 - 1.0f);

  const auto nominator = a2 * static_cast<float>(N_H_above_zero);
  const auto denominator = glm::pi<float>() * ((x + 1.0f) * (x + 1.0f));

  return nominator / denominator;
}

float cr::brdf::visibility(const glm::vec3 &H, const glm::vec3 &V,
                           const glm::vec3 &L, const glm::vec3 &N, float a) {
  const auto H_L = glm::dot(H, L);
  const auto H_V = glm::dot(H, V);

  const auto heaven_H_L = H_L > 0.0f ? 1.0f : 0.0f;
  const auto heaven_H_V = H_V > 0.0f ? 1.0f : 0.0f;

  const auto N_L = glm::dot(N, L);
  const auto N_V = glm::dot(N, V);

  const auto a2 = a * a;
  const auto a2_under_one = 1.0f - a2;

  const auto L_DENOM = glm::abs(N_L) + std::sqrt(a2 + a2_under_one * N_L * N_L);
  const auto V_DENOM = glm::abs(N_V) + std::sqrt(a2 + a2_under_one * N_V * N_V);

  const auto L_H = heaven_H_L / L_DENOM;
  const auto V_H = heaven_H_V / V_DENOM;

  return L_H * V_H;
}

float cr::brdf::diffuse_brdf() { return glm::one_over_pi<float>(); }

float cr::brdf::conductor_fresnal(float f0, float bsdf, float V_H) {
  return bsdf * (f0 + (1.0f - f0) * std::pow(1.0f - glm::abs(V_H), 5.0f));
}

float cr::brdf::fresnal_mix(float ior, float base, float layer, float V_H) {
  const auto f0 = ((1.0f - ior) / (1.0f + ior)) * ((1.0f - ior) / (1.0f + ior));
  const auto fr = f0 + (1.0f - f0) * std::pow(1.0f - glm::abs(V_H), 5.0f);
  return glm::mix(base, layer, fr);
}
