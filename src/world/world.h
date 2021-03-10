#pragma once

#include <array>
#include <unordered_map>

namespace cr
{
    class chunk
    {
    public:
        std::array<u_int16_t, 65536> blocks;
    };

    class world
    {
    public:
        world() = default;

    private:
        std::unordered_map<int32_t , cr::chunk> _chunks;

    };
}    // namespace cr
