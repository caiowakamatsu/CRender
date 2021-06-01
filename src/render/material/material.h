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
            float       emission       = 0;
            glm::vec3   colour;
            std::string name;
        };

        explicit material(information information);

        [[nodiscard]] bool operator==(const cr::material &rhs) const noexcept;

        information info;
    };
}    // namespace cr