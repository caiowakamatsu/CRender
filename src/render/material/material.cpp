#include "material.h"

#include <utility>

cr::material::material(material::information information) : info(std::move(information))
{
}

bool cr::material::operator==(const cr::material &rhs) const noexcept
{
    return info.shade_type == rhs.info.shade_type && info.ior == rhs.info.ior &&
      info.roughness == rhs.info.roughness && info.colour == rhs.info.colour;
}
