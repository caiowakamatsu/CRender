//
// Created by Caio W on 3/2/22.
//

#ifndef CREBON_SCENE_HPP
#define CREBON_SCENE_HPP

#include <optional>

#include <scene/ray.hpp>
#include <scene/configuration.hpp>
#include <material/material.hpp>

namespace cr {
	struct intersection {
		int id;
		float distance;
		cr::material *material;
		glm::vec3 normal;
		glm::vec2 texcoord;
	};

	template<typename T>
	class scene {
	private:
		T *_scene;
	public:
		explicit scene(T *scene) : _scene(scene) {}

		[[nodiscard]] std::optional<cr::intersection> intersect(const cr::ray &ray) {
			return _scene->intersect(ray);
		}
	};
}

#endif //CREBON_SCENE_HPP
