#pragma once

#include <cstdint>
#include <type_traits>

namespace cr
{
    template<class T>
    [[nodiscard]] inline bool is_bit_set(T val, T bit)
    {
//        static_assert(sizeof(T) < bit, "Cannot check if bit is set outside of size of type");
        return (val >> bit) & 1;
    }

    template<class T>
    [[nodiscard]] inline T set_bit(T val, T bit)
    {
//        static_assert(sizeof(T) < bit, "Cannot check if bit is set outside of size of type");
        return val | 1UL << bit;
    }

    template<class T>
    [[nodiscard]] inline unsigned int set_bit_count(T n)
    {
        unsigned int count = 0;
        while (n) {
            count += n & 1;
            n >>= 1;
        }
        return count;
    }

}    // namespace cr
