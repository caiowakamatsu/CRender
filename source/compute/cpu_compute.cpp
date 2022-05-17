//
// Created by Caio W on 2/2/22.
//

#include "cpu_compute.hpp"

cr::compute_resource<bool> cr::cpu_compute::blit(cr::image *target, cr::image source, const glm::ivec2 &coordinate, size_t sample_count) {
	auto resource = cr::compute_resource<bool>();

	resource = _pool->submit([target, source, coordinate, sample_count]() -> bool {
		for (size_t y = 0; y < source.height(); y++) {
			for (size_t x = 0; x < source.width(); x++) {
				const auto coords = glm::ivec2(x, y) + coordinate;
				const auto sample = target->get(coords.x, coords.y);
				const auto adjusted_sample = (sample * static_cast<float>(sample_count) + source.get(x, y)) / static_cast<float>(sample_count + 1);
				target->set(x + coordinate.x, y + coordinate.y, adjusted_sample);
			}
		}
		return true;
	});

	return resource;
}


