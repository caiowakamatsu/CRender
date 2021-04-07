#pragma once

#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

namespace cr::colour
{
    [[nodiscard]] inline glm::vec3 RGB_from_YCbCr(int y, int cb, int cr)
    {
        const auto Y  = static_cast<double>(y);
        const auto Cb = static_cast<double>(cb);
        const auto Cr = static_cast<double>(cr);

        auto r = static_cast<int>((Y + 1.40200 * (Cr - 0x80)));
        auto g = static_cast<int>((Y - 0.34414 * (Cb - 0x80) - 0.71414 * (Cr - 0x80)));
        auto b = static_cast<int>((Y + 1.77200 * (Cb - 0x80)));

        r = std::max(0, std::min(255, r));
        g = std::max(0, std::min(255, g));
        b = std::max(0, std::min(255, b));

        return { r / 255.f, g / 255.f, b / 255.f };
    }
}    // namespace cr::colour
