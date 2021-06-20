#pragma once

#include <vector>
#include <cstdint>
#include <limits>

#include <glm/glm.hpp>

#include <util/exception.h>

namespace cr
{
    class image
    {
    private:
        static constexpr auto ValType_Max = std::numeric_limits<float>::max();

    public:
        image() = default;

        image(const std::vector<float> &data, uint64_t width, uint64_t height)
            : _width(width), _height(height)
        {
            if (data.size() != _width * _height * 4)
                cr::exit("Attempted to create an image with an invalid amount of data");

            _image_data = data;
        }

        image(uint64_t width, uint64_t height) : _width(width), _height(height)
        {
            _image_data = std::vector<float>(_width * _height * 4, ValType_Max);
        }

        void clear()
        {
            std::fill(_image_data.begin(), _image_data.end(), ValType_Max);
        }

        [[nodiscard]] bool valid() const noexcept
        {
            return _width != std::numeric_limits<std::uint64_t>::max() &&
              _height != std::numeric_limits<std::uint64_t>::max();
        }

        [[nodiscard]] float *data() noexcept
        {
            return _image_data.data();
        }

        [[nodiscard]] const float *data() const noexcept
        {
            return _image_data.data();
        }

        [[nodiscard]] uint64_t width() const noexcept
        {
            return _width;
        }

        [[nodiscard]] uint64_t height() const noexcept
        {
            return _height;
        }

        [[nodiscard]] glm::vec3 get_uv(float u, float v) const noexcept
        {
            return get(
              static_cast<uint64_t>(u * _width) % _width,
              static_cast<uint64_t>(v * _height) % _height);
        }

        [[nodiscard]] glm::vec4 get(uint64_t x, uint64_t y) const noexcept
        {
            const auto base_index = (x + y * _width) * 4;

            return {
                _image_data[base_index + 0],
                _image_data[base_index + 1],
                _image_data[base_index + 2],
                _image_data[base_index + 3]
            };
        }

        void set(uint64_t x, uint64_t y, const glm::vec3 &colour) noexcept
        {
            set(x, y, glm::vec4(colour, 1.0f));
        }

        void set(uint64_t x, uint64_t y, const glm::vec4 &colour) noexcept
        {
            const auto base_index = (x + y * _width) * 4;

            _image_data[base_index + 0] = colour.r;
            _image_data[base_index + 1] = colour.g;
            _image_data[base_index + 2] = colour.b;
            _image_data[base_index + 3] = colour.a;
        }

    private:
        std::vector<float> _image_data;
        uint64_t       _width  = std::numeric_limits<uint64_t>::max();
        uint64_t       _height = std::numeric_limits<uint64_t>::max();
    };
}    // namespace cr