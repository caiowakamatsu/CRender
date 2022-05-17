//
// Created by Caio W on 2/2/22.
//

#ifndef CREBON_COMPUTE_DEVICE_HPP
#define CREBON_COMPUTE_DEVICE_HPP

#include <scene/configuration.hpp>
#include <scene/scene.hpp>

#include <concurrentqueue/concurrentqueue.h>

#include <types.hpp>

#include <util/image.hpp>

#include <future>

namespace cr {
	template<typename T>
	class compute_device {
	private:
		T *_device;

	public:
		explicit compute_device(T *device) : _device(device) {}

		using MinMaxPair = std::pair<glm::ivec2, glm::ivec2>;

		template<typename SceneType>
		struct render_parameters {
			uint32_t random_state{};
			uint64_t sample_count{};
			moodycamel::ConcurrentQueue<MinMaxPair> *tasks {};

			cr::atomic_image *buffer{};
			cr::scene<SceneType> *scene;
			cr::scene_configuration *configuration{};
		};

		template <typename SceneType>
		[[nodiscard]] cr::compute_resource<bool> render(render_parameters<SceneType> parameters) {
			return _device->render(parameters);
		}
	};
}

#endif //CREBON_COMPUTE_DEVICE_HPP
