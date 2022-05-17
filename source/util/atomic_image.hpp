//
// Created by Caio W on 12/3/22.
//

#ifndef CREBON_ATOMIC_IMAGE_HPP
#define CREBON_ATOMIC_IMAGE_HPP

#include <vector>
#include <cstdint>
#include <limits>
#include <atomic>
#include <bit>

#include <glm/glm.hpp>

#include <fmt/core.h>

namespace {
	[[nodiscard]] uint32_t float_to_uint32(float f) noexcept {
		auto res = uint32_t();
		std::memcpy(&res, &f, sizeof(uint32_t));
		return res;
	}

	[[nodiscard]] float uint32_to_float(uint32_t u) noexcept {
		auto res = float();
		std::memcpy(&res, &u, sizeof(float));
		return res;
	}

	// Atomically load uint32_t as float
	[[nodiscard]] float load_uint32_atomic_as_float(const uint32_t &val) {
		const auto loaded = __atomic_load_n(&val, __ATOMIC_RELAXED);
		return uint32_to_float(loaded);
	}

	// Atomically store float as uint32_t
	void store_float_as_uint32_atomic(uint32_t &val, float f) {
		const auto u = float_to_uint32(f);
		__atomic_store_n(&val, u, __ATOMIC_RELAXED);
	}

}

namespace cr {
	class atomic_image {
	private:
		static constexpr auto ValType_Max = std::numeric_limits<float>::max();

	public:
		atomic_image() = default;

		atomic_image(const std::vector<float> &data, uint64_t width, uint64_t height)
						: _width(width), _height(height) {
			if (data.size() != _width * _height * 4) {
				fmt::print("failed to create image with [{} {}], data was {}", _width, _height, data.size());
				exit(-1);
			}

			_image_data.reserve(_width * _height * 4);
			for (auto &val: data) {
				_image_data.emplace_back(::float_to_uint32(val));
			}
		}

		atomic_image(uint64_t width, uint64_t height) : _width(width), _height(height) {
			auto float_zero_as_uint32 = ::float_to_uint32(0.0f);
			_image_data = std::vector<uint32_t>(_width * _height * 4, float_zero_as_uint32);
		}

		void clear() {
			const auto max_as_uint32 = float_to_uint32(ValType_Max);
			std::fill(_image_data.begin(), _image_data.end(), max_as_uint32);
		}

		[[nodiscard]] bool valid() const noexcept {
			return _width != std::numeric_limits<std::uint64_t>::max() &&
						 _height != std::numeric_limits<std::uint64_t>::max();
		}

		[[nodiscard]] std::vector<float> data() const noexcept {
			auto data = std::vector<float>();
			data.reserve(_image_data.size());

			for (auto &channel: _image_data)
				data.push_back(::load_uint32_atomic_as_float(channel));

			return data;
		}

		[[nodiscard]] uint64_t width() const noexcept {
			return _width;
		}

		[[nodiscard]] uint64_t height() const noexcept {
			return _height;
		}

		[[nodiscard]] glm::vec4 get_uv(float u, float v) const noexcept {
			return get(
							static_cast<uint64_t>(u * _width) % _width,
							static_cast<uint64_t>(v * _height) % _height);
		}

		[[nodiscard]] glm::vec4 get(uint64_t x, uint64_t y) const noexcept {
			const auto base_index = (x + y * _width) * 4;

			return (*this)[base_index];
		}

		void set(uint64_t x, uint64_t y, const glm::vec3 &colour) noexcept {
			set(x, y, glm::vec4(colour, 1.0f));
		}

		[[nodiscard]] glm::vec4 operator[](size_t index) const noexcept {
			return {
							::load_uint32_atomic_as_float(_image_data[index + 0]),
							::load_uint32_atomic_as_float(_image_data[index + 1]),
							::load_uint32_atomic_as_float(_image_data[index + 2]),
							::load_uint32_atomic_as_float(_image_data[index + 3])
							};
		}

		void set(uint64_t x, uint64_t y, const glm::vec4 &colour) noexcept {
			const auto base_index = (x + y * _width) * 4;

			::store_float_as_uint32_atomic(_image_data[base_index + 0], colour.r);
			::store_float_as_uint32_atomic(_image_data[base_index + 1], colour.g);
			::store_float_as_uint32_atomic(_image_data[base_index + 2], colour.b);
			::store_float_as_uint32_atomic(_image_data[base_index + 3], colour.a);
		}

	private:
		// floats stored as uint32_t because atomic loads
		std::vector<uint32_t> _image_data;
		uint64_t _width = std::numeric_limits<uint64_t>::max();
		uint64_t _height = std::numeric_limits<uint64_t>::max();
	};
}

#endif //CREBON_ATOMIC_IMAGE_HPP
