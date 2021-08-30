#pragma once

#include <cstdint>

namespace cr
{
    class random
    {
    public:
        explicit random(std::uint32_t state) : _state(state) { }

        [[nodiscard]] float next_float()
        {
            _state ^= (_state << 13);
            _state ^= (_state >> 17);
            _state ^= (_state << 5);
            return _state / 4294967296.0f;
        }

        [[nodiscard]] std::uint32_t next_int(std::uint32_t max)
        {
            return std::uint32_t(next_float() * max);
        }

    private:
        std::uint32_t _state;
    };
}    // namespace cr