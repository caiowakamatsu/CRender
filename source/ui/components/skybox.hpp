//
// Created by howto on 25/5/2022.
//

#ifndef CREBON_SKYBOX_HPP
#define CREBON_SKYBOX_HPP

#include <glm/glm.hpp>
#include <numbers>

namespace cr::component {
[[nodiscard]] inline glm::vec3 ComputeWaveLambdaRayleigh(const glm::vec3 &lambda)
{
  const float n = 1.0003f;
  const float N = 2.545E25;
  const float pn = 0.035f;
  const float n2 = n * n;
  const float pi3 = std::numbers::pi_v<float> * std::numbers::pi_v<float> * std::numbers::pi_v<float>;
  const float rayleighConst = (8.0f * pi3 * pow(n2 - 1.0f,2.0f)) / (3.0f * N) * ((6.0f + 3.0f * pn) / (6.0f - 7.0f * pn));
  return rayleighConst / (lambda * lambda * lambda * lambda);
}

class skybox {
public:
  struct Options {
    float sunRadius = 500.0f;
    float sunRadiance = 20.0f;

    float mieG = 0.76f;
    float mieHeight = 1200.0;

    float rayleighHeight = 8000.0f;

    float earthRadius = 6360000.0f;
    float earthAtmTopRadius = 6420000.0f;

    glm::vec2 sun_pos = glm::vec2(0.5, 0.7);

    glm::vec3 earthCenter = glm::vec3(0, - 6360000.0f, 0);
    glm::vec3 waveLambdaMie = glm::vec3(2e-7f);
    glm::vec3 waveLambdaOzone = glm::vec3(1.36820899679147, 3.31405330400124, 0.13601728252538) * 0.6e-6f * 2.504f;
    glm::vec3 waveLambdaRayleigh = ComputeWaveLambdaRayleigh({680e-9, 550e-9, 450e-9});
  };
  struct DisplayContents {
  };

  struct Component {
    [[nodiscard]] skybox::Options display(DisplayContents contents) const;
  };
};
}

#endif // CREBON_SKYBOX_HPP
