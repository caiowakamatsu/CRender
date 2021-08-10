#pragma once

#include <OpenImageDenoise/oidn.hpp>

#include <objects/image.h>
#include <util/asset_loader.h>

namespace cr
{
    [[nodiscard]] inline cr::image denoise(
      const cr::image *            framebuffer_in,
      const cr::image *            normals,
      const cr::image *            albedo,
      cr::asset_loader::image_type output_type)
    {
        const auto width  = framebuffer_in->width();
        const auto height = framebuffer_in->height();

        auto colour_buffer   = framebuffer_in->as_float3_buffer();
        auto normal_buffer   = normals->as_float3_buffer();
        auto albedo_buffer   = albedo->as_float3_buffer();
        auto denoised_buffer = std::vector<float>(width * height * 3);

        auto device = oidn::newDevice();
        device.commit();

        auto filter = device.newFilter("RT");
        filter.setImage("color", colour_buffer.data(), oidn::Format::Float3, width, height);
        filter.setImage("normal", normal_buffer.data(), oidn::Format::Float3, width, height);
        filter.setImage("albedo", albedo_buffer.data(), oidn::Format::Float3, width, height);
        filter.setImage("output", denoised_buffer.data(), oidn::Format::Float3, width, height);
        filter.set("hdr", true);

        filter.commit();
        filter.execute();

        const char *errorMessage;
        if (device.getError(errorMessage) != oidn::Error::None)
            cr::logger::error("There was an error denoising image: [{}]", errorMessage);

        return cr::image::from_float3_buffer(denoised_buffer, width, height);
    }
}    // namespace cr
