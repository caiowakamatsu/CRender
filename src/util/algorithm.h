#pragma once

#include <vector>
#include <functional>

namespace cr::algorithm
{
    template<typename T>
    [[nodiscard]] inline uint64_t index_or_push_back(std::vector<T> vector, T object)
    {
        for (auto i = 0; i < vector.size(); i++)
            if (vector[i] == object) return i;
        vector.push_back(std::move(object));

        return vector.size() - 1;
    }

    template<typename T>
    [[nodiscard]] inline std::vector<size_t> find_string_matches(
      const std::string &                           source,
      const std::vector<T> &                        objects,
      std::function<std::string (const T &)> string_fetch)
    {
        auto indices = std::vector<size_t>();
        indices.reserve(objects.size());

        for (auto i = 0; i < objects.size(); i++)
        {
            const auto id = string_fetch(objects[i]);
            if (id.find(source) != std::string::npos)
                indices.push_back(i);
        }

        return indices;
    }

    template <typename T>
    [[nodiscard]] inline std::vector<size_t>
      find_string_matches(const std::string &source, const std::vector<T> &objects)
    {
        auto indices = std::vector<size_t>();
        indices.reserve(objects.size());

        for (auto i = 0; i < objects.size(); i++)
            if (source.empty() || source.find(objects[i]) != std::string::npos)
                indices.push_back(i);

        return indices;
    }
}    // namespace cr::algorithm
