//
// Created by Caio W on 5/2/22.
//

#ifndef CREBON_MATERIAL_HPP
#define CREBON_MATERIAL_HPP

#include <scene/ray.hpp>

#include <util/sampling.hpp>
#include <util/image.hpp>

#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>
#include "brdf.hpp"

#include <material/sampleable.hpp>

namespace cr
{
	struct evaluted_ray
	{
		float pdf;
		cr::ray ray;
	};

	struct evaluation
	{
		glm::vec3 bxdf;
		glm::vec3 emission;
	};

	class material {
	public:
		enum class type
		{
			gltf,
			diffuse,
			metal,
		};

		[[nodiscard]] virtual evaluted_ray evalute_ray(const glm::vec3 &incident, const glm::vec3 &normal, const glm::vec3 &point, cr::random* random) const noexcept = 0;

		[[nodiscard]] virtual evaluation evalute(const glm::vec3 &incident, const glm::vec3 &outgoing, const glm::vec3 &normal, const glm::vec2 &uv) const noexcept = 0;

		[[nodiscard]] virtual glm::vec3 emission(const glm::vec2 &uv) const noexcept = 0;

		//[[nodiscard]] virtual type material_type() const noexcept = 0;
	};

	class gltf_material : public material {
	private:
		cr::sampleable<float> _roughness;
		cr::sampleable<float> _metalness;
		cr::sampleable<glm::vec3> _emissiveness;
		cr::sampleable<glm::vec3> _base_colour;

	public:
		gltf_material(cr::sampleable<float> roughness, cr::sampleable<float> metalness, cr::sampleable<glm::vec3> emissiveness, cr::sampleable<glm::vec3> base_colour) noexcept :
						_roughness(std::move(roughness)),
						_metalness(std::move(metalness)),
						_emissiveness(std::move(emissiveness)),
						_base_colour(std::move(base_colour)) {}

		[[nodiscard]] evaluted_ray evalute_ray(const glm::vec3 &incident, const glm::vec3 &normal, const glm::vec3 &point, cr::random* random) const noexcept override {
			auto cos_hemp_dir = cr::sampling::cosine_hemisphere(
							normal,
							glm::vec2(random->next_float(), random->next_float()));

			const auto direction = glm::normalize(cos_hemp_dir);

			return {
							// If a direction is at a grazing angle with the normal, the pdf is zero. This is a special case. We can't use the cosine of the angle to calculate the pdf.
							.pdf = glm::max(glm::dot(direction, normal) * glm::one_over_pi<float>(), 0.001f),
							.ray = cr::ray(point + normal * 0.01f, direction)
			};
		}

		[[nodiscard]] evaluation evalute(const glm::vec3 &incident, const glm::vec3 &outgoing, const glm::vec3 &normal, const glm::vec2 &uv) const noexcept override {
			const auto V = -incident; // From shading location -> eye
			const auto L = outgoing; // From shading location -> light
			const auto N = normal; // Normal at shading location
			const auto H = glm::normalize(V + L); // Half vector

			const auto base_color = _base_colour.sample(uv.x, uv.y);
			const auto emission = glm::vec3(0.0f);

			const auto bxdf = cr::brdf::gltf_brdf(
							V, L, N, H, base_color, _roughness.sample(uv.x, uv.y), _metalness.sample(uv.x, uv.y)
			);
			return {
				.bxdf = bxdf,
				.emission = glm::vec3(emission)
			};
		}

		[[nodiscard]] glm::vec3 emission(const glm::vec2 &uv) const noexcept override {
			return _base_colour.sample(uv.x, uv.y);
		}
	};
}

#endif //CREBON_MATERIAL_HPP