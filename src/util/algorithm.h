#pragma once

#include <vector>

namespace cr::algorithm
{
    template <typename T>
    [[nodiscard]] inline uint64_t index_or_push_back(std::vector<T> vector, T object)
    {
        for (auto i = 0; i < vector.size(); i++)
            if (vector[i] == object) return i;
        vector.push_back(std::move(object));

        return vector.size() - 1;
    }
}
