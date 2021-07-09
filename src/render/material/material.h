#pragma once

#include <string>
#include <optional>

#include <glm/glm.hpp>
#include <objects/image.h>

namespace cr
{
    class material
    {
    public:
        enum type : unsigned char
        {
            metal,
            smooth,
            glass
        };

        [[nodiscard]] static std::string get_type_name(type type)
        {
            switch (type)
            {
            case metal: return "Metal";
            case smooth: return "Smooth";
            case glass: return "Glass";
            }
        }

        struct information
        {
            type                     type           = smooth;
            float                    ior            = 1.5;
            float                    roughness      = 0.5;
            float                    reflectiveness = 1;
            float                    emission       = 0;
            glm::vec4                colour         = glm::vec4(1, 1, 1, 1);
            std::string              name           = "ERROR - Report";
            std::optional<uint32_t> tex;
        };

        material() { info = information(); }

        explicit material(information information);

        [[nodiscard]] bool operator==(const cr::material &rhs) const noexcept;

        information info;
    };
}    // namespace cr
