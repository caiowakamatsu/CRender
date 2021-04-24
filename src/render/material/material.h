#pragma once

#include <string>

#include <glm/glm.hpp>

namespace cr
{
    class material
    {
    public:
        enum type : unsigned char
        {
            metal,
            smooth,
        };

        [[nodiscard]] static std::string get_type_name(type type)
        {
            switch (type)
            {
            case metal: return "Metal";
            case smooth: return "Smooth";
            }
        }

        struct information
        {
            type        type;
            float       ior            = 0;
            float       roughness      = 0;
            float       reflectiveness = 0;
            glm::vec3   colour;
            std::string name;
        };

        explicit material(const information &information);

        [[nodiscard]] glm::vec3 colour() const noexcept;

        [[nodiscard]] bool operator==(const cr::material &rhs) const noexcept;

        [[nodiscard]] std::string name() const noexcept;

        [[nodiscard]] cr::material::type mat_type() const noexcept;

        type      _material_type;
        float     _ior;
        float     _roughness;
        glm::vec3 _base_colour;

    private:
        std::string _name;
    };
}    // namespace cr
