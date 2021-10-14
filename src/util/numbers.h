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
    public:
        inline constexpr static float e   = 2.71828182845f;
        inline constexpr static float pi  = 3.14159265359f;
        inline constexpr static float half_pi = 3.14159265359f / 2.0f;
        inline constexpr static float tau = 6.28318530717f;

        inline constexpr static float sqrt_3 = 1.732050808f;

        inline constexpr static float inv_e   = 1.0f / 2.71828182845f;
        inline constexpr static float inv_pi  = 1.0f / 3.14159265359f;
        inline constexpr static float two_over_pi = 2.0f / 3.14159265359f;
        inline constexpr static float inv_tau = 1.0f / 6.28318530717f;
    };

    template<>
    class numbers<double>
    {
    public:
        inline constexpr static double e   = 2.71828182845;
        inline constexpr static double pi  = 3.14159265359;
        inline constexpr static double half_pi = 3.14159265359 / 2.0;
        inline constexpr static double tau = 6.28318530717;

        inline constexpr static double sqrt_3 = 1.732050808;

        inline constexpr static double inv_e   = 1.0 / 2.71828182845;
        inline constexpr static double inv_pi  = 1.0 / 3.14159265359;
        inline constexpr static double two_over_pi = 2.0 / 3.14159265359;
        inline constexpr static double inv_tau = 1.0 / 6.28318530717;
    };

}    // namespace cr
