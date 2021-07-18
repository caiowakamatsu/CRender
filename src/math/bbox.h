#pragma once

#include <glm/common.hpp>
#include <embree3/rtcore.h>

namespace cr
{
    class bbox
    {
    public:
        bbox() : min(0), max(0) {};

        bbox(const glm::vec3 &min, const glm::vec3 &max) : min(min), max(max) { }

        bbox(const RTCBuildPrimitive &primitive)
            : min(primitive.lower_x, primitive.lower_y, primitive.lower_z),
              max(primitive.upper_x, primitive.upper_y, primitive.upper_z)
        {
        }

        bbox(const RTCBounds &bounds)
            : min(bounds.lower_x, bounds.lower_y, bounds.lower_z),
              max(bounds.upper_x, bounds.upper_y, bounds.upper_z)
        {
        }

        void assert_nonzero()
        {
//            assert(min != glm::vec3(0) && max != glm::vec3(0));/
        }

        [[nodiscard]] glm::vec3 size() const noexcept { return max - min; }

        [[nodiscard]] float area() const noexcept
        {
            const auto box_size = size();
            const auto xy       = box_size.x * box_size.y;
            const auto xz       = box_size.x * box_size.z;
            const auto yz       = box_size.y * box_size.z;
            return (xy + xz + yz) * 2;
        }

        [[nodiscard]] bbox merge(const bbox &rhs) const noexcept
        {
            auto ret = bbox(glm::vec3(
                          glm::min(min.x, rhs.min.x),
                          glm::min(min.y, rhs.min.y),
                          glm::min(min.z, rhs.min.z)),
                        glm::vec3(
                          glm::max(max.x, rhs.max.x),
                          glm::max(max.y, rhs.max.y),
                          glm::max(max.z, rhs.max.z)));
            ret.assert_nonzero();
            return ret;
        }

        glm::vec3 min = glm::vec3(999.f, 999.f, 999.f);
        glm::vec3 max = glm::vec3(999.f, 999.f, 999.f);
    };
}    // namespace cr
