#pragma once

#include <glm/glm.hpp>
#include <objects/image.h>

namespace cr
{
    class sky
    {
    public:
        enum class mode
        {
            solid_colour,
            sky_box,
        };

        [[nodiscard]] glm::vec3 sample(float u, float v) const noexcept {
            switch (current_mode)
            {
            case cr::sky::mode::solid_colour:
                return colour;
                break;
            
            case cr::sky::mode::sky_box:
                return skybox.get_uv(u, v);
                break;
            }
        }

        mode current_mode;
        glm::vec3 colour;
        cr::image skybox;
    };
} // namespace cr
