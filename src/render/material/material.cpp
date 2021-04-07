#include "material.h"

cr::material::material(const material::information& information)
    : _material_type(information.type), _ior(information.ior), _roughness(information.roughness),
      _base_colour(information.colour), _name(information.name), _emission(information.emission)
{
}

glm::vec3 cr::material::colour() const noexcept
{
    return _base_colour;
}

bool cr::material::operator==(const cr::material &rhs) const noexcept
{
    return _material_type == rhs._material_type && _ior == rhs._ior &&
      _roughness == rhs._roughness && _base_colour == rhs._base_colour;
}
std::string cr::material::name() const noexcept
{
    return _name;
}

cr::material::type cr::material::mat_type() const noexcept
{
    return _material_type;
}
