#pragma once

namespace cr
{
    template<typename T>
    class numbers
    {
    };

    template<>
    class numbers<float>
    {
        inline constexpr static float e   = 2.71828182845f;
        inline constexpr static float pi  = 3.14159265359f;
        inline constexpr static float tau = 6.28318530717f;

        inline constexpr static float inv_e   = 1.0f / 2.71828182845f;
        inline constexpr static float inv_pi  = 1.0f / 3.14159265359f;
        inline constexpr static float inv_tau = 1.0f / 6.28318530717f;
    };

    template<>
    class numbers<double>
    {
        inline constexpr static double e   = 2.71828182845;
        inline constexpr static double pi  = 3.14159265359;
        inline constexpr static double tau = 6.28318530717;

        inline constexpr static double inv_e   = 1.0 / 2.71828182845;
        inline constexpr static double inv_pi  = 1.0 / 3.14159265359;
        inline constexpr static double inv_tau = 1.0 / 6.28318530717;
    };

}    // namespace cr
