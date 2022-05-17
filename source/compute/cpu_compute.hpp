//
// Created by Caio W on 2/2/22.
//

#ifndef CREBON_CPU_COMPUTE_HPP
#define CREBON_CPU_COMPUTE_HPP

#include <types.hpp>
#include <compute/compute_device.hpp>

#include <future>

#include <thread-pool/thread_pool.hpp>

#include <material/brdf.hpp>

namespace cr {
	class cpu_compute {
	private:
		thread_pool *_pool;
	public:
		explicit cpu_compute(thread_pool *pool) : _pool(pool) {}

		template <typename T>
		using render_parameters = compute_device<cpu_compute>::render_parameters<T>;

		template <typename SceneType>
		[[nodiscard]] cr::compute_resource<bool> render(render_parameters<SceneType> parameters) {
			auto resource = cr::compute_resource<bool>();

			resource = _pool->template submit([parameters]() -> bool {
				auto current = cr::compute_device<cpu_compute>::MinMaxPair();
				while (parameters.tasks->template try_dequeue(current)) {
					const auto size = current.second - current.first;
					auto random = cr::random(parameters.random_state);

					for (auto y = current.first.y; y < current.second.y; y++) {
						for (auto x = current.first.x; x < current.second.x; x++) {
							auto throughput = glm::vec3(1.0f);
							auto final = glm::vec3(0.0f);

							// Do primary ray manually
							auto ray = parameters.configuration->ray(x, y, random);

							struct vertex_data {
								float cos_theta;
								float pdf;
								glm::vec3 bxdf;
							};
							auto current_vert = vertex_data();

							auto had_intersection = false;
							{
								const auto intersection = parameters.scene->intersect(ray);

								if (intersection.has_value()) {
									const auto &isect = intersection.value();
									const auto point = ray.at(isect.distance);

									const auto material = isect.material;

									const auto ray_evaluation = material->evalute_ray(ray.direction, isect.normal, point, &random);
									const auto evaluted = material->evalute(ray.direction, ray_evaluation.ray.direction, isect.normal, isect.texcoord);
									const auto cos_theta = glm::dot(ray_evaluation.ray.direction, isect.normal);

									current_vert.cos_theta = cos_theta;
									current_vert.pdf = ray_evaluation.pdf;
									current_vert.bxdf = evaluted.bxdf;

									final = material->emission(isect.texcoord);
//									final = isect.normal * 0.5f + 0.5f;

									ray = ray_evaluation.ray;

									had_intersection = true;
								} else {
									const auto t = ray.direction.y * 0.5f + 0.5f;
									const auto sky = glm::mix(
													glm::vec3(0.2f, 0.2f, 0.8f),
													glm::vec3(1.0f, 1.0f, 1.0f), t);

									final = sky;
								}
							}

							if (had_intersection) {
								for (size_t i = 1; i < parameters.configuration->bounces(); i++) {
									const auto intersection = parameters.scene->intersect(ray);
									throughput *= current_vert.bxdf * current_vert.cos_theta / current_vert.pdf;

									if (intersection.has_value()) {
										const auto &isect = intersection.value();
										const auto point = ray.at(isect.distance);

										const auto material = intersection.value().material;

										const auto ray_evaluation = material->evalute_ray(ray.direction, isect.normal, point,
																																			&random);
										const auto evaluted = material->evalute(ray.direction, ray_evaluation.ray.direction, isect.normal,
																														isect.texcoord);

										const auto cos_theta = glm::dot(isect.normal, ray_evaluation.ray.direction);
//										final += throughput * material->emission(isect.texcoord);
										final += throughput;

										current_vert.cos_theta = cos_theta;
										current_vert.pdf = ray_evaluation.pdf;
										current_vert.bxdf = evaluted.bxdf;

										ray = ray_evaluation.ray;
									} else {
										const auto t = ray.direction.y * 0.5f + 0.5f;
										const auto sky = glm::mix(
														glm::vec3(0.2f, 0.2f, 0.8f),
														glm::vec3(1.0f, 1.0f, 1.0f), t);

										final += throughput * sky;
										break;
									}
								}
							}


							// if nan discard the pixel
							if (!(std::isnan(final.x) || std::isnan(final.y) || std::isnan(final.z))) {
								const auto sample = glm::vec3(parameters.buffer->get(x, y)) * static_cast<float>(parameters.sample_count);
								auto averaged = glm::vec4((sample + final) / (parameters.sample_count + 1.0f), 1.0f); // 1.0f implicitlly promotes to float
								parameters.buffer->set(x, y, averaged);
							}
						}
					}
				}

				return true;
			});

			return resource;
		}

		[[nodiscard]] cr::compute_resource<bool> blit(cr::image *target, cr::image source, const glm::ivec2 &coordinate, size_t sample_count);
	};
}

#endif //CREBON_CPU_COMPUTE_HPP
