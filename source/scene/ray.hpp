//
// Created by Caio W on 3/2/22.
//

#ifndef CREBON_RAY_HPP
#define CREBON_RAY_HPP

#include <glm/glm.hpp>

namespace cr {
	class ray {
	public:
		glm::vec3 origin;
		glm::vec3 direction;

		ray() = default;

		ray(const glm::vec3 &origin, const glm::vec3 &direction) : origin(origin), direction(direction) {}

		[[nodiscard]] glm::vec3 at(float distance) const noexcept
		{
			return origin + direction * distance;
		}
	};
}

#endif //CREBON_RAY_HPP
