#pragma once

#include <glm/glm.hpp>

namespace
{
    [[nodiscard]] float randf() noexcept;
}

namespace cr::sampling
{
    namespace cook_torrence
    {
        /**
         * Specular D (Normal Distribution Function)
         *
         *                          a ^ 2
         * D(h,a) = -----------------------------------
         *          pi((n * h) ^ 2 (a ^ 2 - 1) + 1) ^ 2
         *
         */
        [[nodiscard]] inline float
          specular_d(float NoH, float roughness)
        {
            constexpr auto pi = 3.141592f;

            const auto a2 = roughness * roughness;
            const auto d = ((NoH * a2 - NoH) * NoH + 1.0f);

            return a2 / (d * d * pi);
        }

        /**
         * Specular G (Geometric Shadowing)
         *
         *                                                          0.5
         * V(v,l,a) = -----------------------------------------------------------------------------------------
         *            n * l sqrt((n * v) ^ 2 (1 - a ^ 2) + a ^ 2) + n * v sqrt((n * l) ^ 2 (1 - a ^ 2) + a ^ 2)
         *
         */
        [[nodiscard]] inline float specular_g(float NoV, float NoL, float roughness)
        {
            const auto a2 = roughness * roughness;

            const auto ggxv = NoL * glm::sqrt(NoV * NoV * (1.0f - a2) + a2);
            const auto ggxl = NoV * glm::sqrt(NoL * NoL * (1.0f - a2) + a2);

            return 0.5f / (ggxv + ggxl);
        }

        /**
         * Specular F (Fresnel)
         *
         * F(v,h,f0,f90) = f0 + (f90 - f0) (1 - v * h) ^ 5
         *
         */
        [[nodiscard]] inline glm::vec3 specular_f(float u, float f0, float f90)
        {
            return f0 + (glm::vec3(f90, f90, f90) - f0) * glm::pow(1.0f - u, 5.0f);
        }

        /**
         * Specular F (Fresnel)
         *
         * F(v,h,f0,f90) = f0 + (f90 - f0) (1 - v * h) ^ 5
         *
         */
        [[nodiscard]] inline glm::vec3 specular_f(float u, const float f0)
        {
            const auto f = glm::pow(1.0f - u, 5.0f);
            return glm::vec3(f + f0 * (1.0f - f));
        }

    }    // namespace cook_torrence

    [[nodiscard]] inline glm::vec3 hemp_rand()
    {
        while (true)
        {
            auto point = glm::vec3(
              ::randf() * 2 - 1,
              ::randf() * 2 - 1,
              ::randf() * 2 - 1);

            if (glm::length2(point) >= 1) continue;

            return glm::normalize(point);
        }
    }

    [[nodiscard]] inline glm::vec3 sphere(const glm::vec2 uv)
    {
        const auto cos_theta = 2.0f * uv.x - 1.0f;
        const auto sin_theta = glm::sqrt(1.0f - cos_theta * cos_theta);

        const auto phi = 2.0f * 3.1415f * uv.y;
        const auto sin_phi = glm::sin(phi);
        const auto cos_phi = glm::cos(phi);

        return glm::vec3(sin_theta * cos_phi, cos_theta, sin_theta * sin_phi);
    }

    [[nodiscard]] inline glm::vec3 hemp_cos(const glm::vec3 normal, const glm::vec2 uv)
    {
        const auto p = sphere(uv);
        return normal + p;
    }

    [[nodiscard]] inline glm::vec3 cos_hemp(const float x, const float y)
    {
        const auto r     = sqrtf(x);
        const auto theta = 6.283f * y;

        const auto u = r * cosf(theta);
        const auto v = r * sinf(theta);

        return { u, v, sqrtf(fmaxf(0.0f, 1.0f - x)) };
    }
}    // namespace cr::sampling
