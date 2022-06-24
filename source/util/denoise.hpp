//
// Created by Caio W on 12/6/22.
//

#ifndef CREBON_DENOISE_HPP
#define CREBON_DENOISE_HPP

#include <OpenImageDenoise/oidn.hpp>
#include <util/atomic_image.hpp>

namespace cr
{
[[nodiscard]] inline std::vector<float> denoise(
    cr::image_view *framebuffer,
    cr::image_view *normal,
    cr::image_view *albedo
    ) {
  const auto width = framebuffer->width;
  const auto height = framebuffer->height;

  auto denoised_buffer = std::vector<float>(width * height * 3);

  auto device = oidn::newDevice();
  device.commit();

  auto filter = device.newFilter("RT");
  filter.setImage("color", framebuffer->data.data(), oidn::Format::Float3, width, height);
  filter.setImage("normal", normal->data.data(), oidn::Format::Float3, width, height);
  filter.setImage("albedo", albedo->data.data(), oidn::Format::Float3, width, height);
  filter.setImage("output", denoised_buffer.data(), oidn::Format::Float3, width, height);
  filter.set("hdr", true);

  filter.commit();
  filter.execute();

  const char *errorMessage;
  if (device.getError(errorMessage) != oidn::Error::None)
    throw std::runtime_error(fmt::format("failed to denoise {}", errorMessage));

  return denoised_buffer;
}

}

#endif // CREBON_DENOISE_HPP
