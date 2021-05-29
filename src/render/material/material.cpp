#include "material.h"

#include <utility>

cr::material::material(material::information information) : info(std::move(information))
{
}

bool cr::material::operator==(const cr::material &rhs) const noexcept
{
    return info.type == rhs.info.type && info.ior == rhs.info.ior &&
      info.roughness == rhs.info.roughness && info.colour == rhs.info.colour;
}
