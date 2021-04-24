#pragma once

#include <vector>
#include <cstdint>
#include <limits>

#include <glm/glm.hpp>

namespace cr
{
    class image
    {
    public:
        image() = default;

        image(const std::vector<uint32_t> &data, uint64_t width, uint64_t height);

        image(uint64_t width, uint64_t height);

        void clear();

        [[nodiscard]] bool valid() const noexcept;

        [[nodiscard]] uint32_t *data() noexcept;

        [[nodiscard]] uint64_t width() const noexcept;

        [[nodiscard]] uint64_t height() const noexcept;

        [[nodiscard]] glm::vec3 get_uv(float u, float v) const noexcept;

        [[nodiscard]] glm::vec3 get(uint64_t x, uint64_t y) const noexcept;

        void set(uint64_t x, uint64_t y, const glm::vec3 &colour) noexcept;

    private:
        std::vector<uint32_t> _image_data;
        uint64_t              _width  = std::numeric_limits<uint64_t>::max();
        uint64_t              _height = std::numeric_limits<uint64_t>::max();
    };
}    // namespace cr