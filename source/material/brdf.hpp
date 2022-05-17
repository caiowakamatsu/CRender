//
// Created by Caio W on 14/3/22.
//

#ifndef CREBON_BRDF_HPP
#define CREBON_BRDF_HPP

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace cr::brdf {
	[[nodiscard]] glm::vec3 gltf_brdf(const glm::vec3 &V, const glm::vec3 &L, const glm::vec3 &N, const glm::vec3 &H, const glm::vec3 &base_colour, const float &roughness, const float &metalness) noexcept;

	[[nodiscard]] float specular_brdf(const glm::vec3 &N, const glm::vec3 &L, const glm::vec3 &V, const glm::vec3 &H, float a);

	/*
	 * D = (a^2 * _x+(N*H)) / pi((N * H)^2 * (a^2 - 1) + 1)^2
	 */
	[[nodiscard]] float ggx_microfacet(const glm::vec3 &N, const glm::vec3 &H, float a);

	/**
	 *                                                          0.5
	 * V(v,l,a) =
	 * -----------------------------------------------------------------------------------------
	 *            n * l sqrt((n * v) ^ 2 (1 - a ^ 2) + a ^ 2) + n * v sqrt((n * l) ^ 2 (1 - a ^
	 * 2) + a ^ 2)
	 *
	 */
	 [[nodiscard]] float visibility(const glm::vec3 &H, const glm::vec3 &V, const glm::vec3 &L, const glm::vec3 &N, float a);

	 [[nodiscard]] float diffuse_brdf();

	 [[nodiscard]] float conductor_fresnal(float f0, float bsdf, float V_H);

	 [[nodiscard]] float fresnal_mix(float ior, float base, float layer, float V_H);
}

#endif //CREBON_BRDF_HPP
