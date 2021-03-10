#include "image.h"

cr::image::image(const std::vector<uint32_t> &data, uint64_t width, uint64_t height)
    : _width(width), _height(height)
{
    _image_data = data;
}

bool cr::image::valid() const noexcept
{
    return _width != std::numeric_limits<uint64_t>::max() &&
      _height != std::numeric_limits<uint64_t>::max();
}

uint32_t *cr::image::data() noexcept
{
    return _image_data.data();
}

uint64_t cr::image::width() const noexcept
{
    return _width;
}

uint64_t cr::image::height() const noexcept
{
    return _height;
}

glm::vec3 cr::image::get_uv(float u, float v) const noexcept
{
    const auto x = static_cast<uint64_t>(u * _width);
    const auto y = static_cast<uint64_t>(v * _height);

    return get(x, y);
}

glm::vec3 cr::image::get(uint64_t x, uint64_t y) const noexcept
{
    const auto packed = _image_data[x + y * _width];

    [[maybe_unused]] const auto alpha = ((packed >> 24) & 0xFF) / 255.f;
    const auto                  red   = ((packed >> 16) & 0xFF) / 255.f;
    const auto                  green = ((packed >> 8) & 0xFF) / 255.f;
    const auto                  blue  = ((packed) & 0xFF) / 255.f;

    return glm::vec3(red, green, blue);
}
