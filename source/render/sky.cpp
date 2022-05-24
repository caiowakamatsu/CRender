//
// Created by howto on 25/5/2022.
//

#include "sky.hpp"

#include <numbers>

namespace {
#define EPSILON 1e-5f

#define SAMPLES_NUMS 16
using namespace glm;
using float2 = glm::vec2;
using float3 = glm::vec3;
using float4 = glm::vec4;
float saturate(float x) { return clamp(x, 0.0f, 1.0f); }

float3 ComputeSphereNormal(float2 coord, float phiStart, float phiLength,
                           float thetaStart, float thetaLength) {
  float3 normal;
  normal.x = -sin(thetaStart + coord.y * thetaLength) *
             sin(phiStart + coord.x * phiLength);
  normal.y = -cos(thetaStart + coord.y * thetaLength);
  normal.z = -sin(thetaStart + coord.y * thetaLength) *
             cos(phiStart + coord.x * phiLength);
  return normalize(normal);
}

float2 ComputeRaySphereIntersection(float3 position, float3 dir, float3 center,
                                    float radius) {
  float3 origin = position - center;
  float B = dot(origin, dir);
  float C = dot(origin, origin) - radius * radius;
  float D = B * B - C;

  float2 minimaxIntersections;
  if (D < 0.0) {
    minimaxIntersections = float2(-1.0, -1.0);
  } else {
    D = sqrt(D);
    minimaxIntersections = float2(-B - D, -B + D);
  }

  return minimaxIntersections;
}

float ComputePhaseMie(float theta, float g) {
  float g2 = g * g;
  return (1.0f - g2) / pow(1.0f + g2 - 2.0f * g * saturate(theta), 1.5f) /
         (4.0f * std::numbers::pi_v<float>);
}

float ComputePhaseRayleigh(float theta) {
  float theta2 = theta * theta;
  return (theta2 * 0.75f + 0.75f) / (4.0f * std::numbers::pi_v<float>);
}

float ChapmanApproximation(float X, float h, float cosZenith) {
  float c = sqrt(X + h);
  float c_exp_h = c * exp(-h);

  if (cosZenith >= 0.0) {
    return c_exp_h / (c * cosZenith + 1.0f);
  } else {
    float x0 = sqrt(1.0f - cosZenith * cosZenith) * (X + h);
    float c0 = sqrt(x0);

    return 2.0f * c0 * exp(X - x0) - c_exp_h / (1.0f - c * cosZenith);
  }
}

float GetOpticalDepthSchueler(float h, float H, float earthRadius,
                              float cosZenith) {
  return H * ChapmanApproximation(earthRadius / H, h / H, cosZenith);
}

float3 GetTransmittance(cr::component::skybox::Options setting, float3 L,
                        float3 V) {
  float ch = GetOpticalDepthSchueler(L.y, setting.rayleighHeight,
                                     setting.earthRadius, V.y);
  return exp(-(setting.waveLambdaMie + setting.waveLambdaRayleigh) * ch);
}

float2 ComputeOpticalDepth(cr::component::skybox::Options setting,
                           float3 samplePoint, float3 V, float3 L, float neg) {
  float rl = length(samplePoint);
  float h = rl - setting.earthRadius;
  float3 r = samplePoint / rl;

  float cos_chi_sun = dot(r, L);
  float cos_chi_ray = dot(r, V * neg);

  float opticalDepthSun = GetOpticalDepthSchueler(
      h, setting.rayleighHeight, setting.earthRadius, cos_chi_sun);
  float opticalDepthCamera =
      GetOpticalDepthSchueler(h, setting.rayleighHeight, setting.earthRadius,
                              cos_chi_ray) *
      neg;

  return float2(opticalDepthSun, opticalDepthCamera);
}

void AerialPerspective(cr::component::skybox::Options setting, float3 start,
                       float3 end, float3 V, float3 L, bool infinite,
                       float3 &transmittance, float3 &insctrMie,
                       float3 &insctrRayleigh) {
  float inf_neg = infinite ? 1.0 : -1.0;

  float3 sampleStep = (end - start) / float(SAMPLES_NUMS);
  float3 samplePoint = end - sampleStep;
  float3 sampleLambda = setting.waveLambdaMie + setting.waveLambdaRayleigh +
                        setting.waveLambdaOzone;

  float sampleLength = length(sampleStep);

  auto scattering = float3(0.0);
  float2 lastOpticalDepth = ComputeOpticalDepth(setting, end, V, L, inf_neg);

  for (int i = 1; i < SAMPLES_NUMS; i++, samplePoint -= sampleStep) {
    float2 opticalDepth =
        ComputeOpticalDepth(setting, samplePoint, V, L, inf_neg);

    float3 segment_s =
        exp(-sampleLambda * (opticalDepth.x + lastOpticalDepth.x));
    float3 segment_t =
        exp(-sampleLambda * (opticalDepth.y - lastOpticalDepth.y));

    transmittance *= segment_t;

    scattering = scattering * segment_t;
    scattering += exp(-(length(samplePoint) - setting.earthRadius) /
                      setting.rayleighHeight) *
                  segment_s;

    lastOpticalDepth = opticalDepth;
  }

  insctrMie = scattering * setting.waveLambdaMie * sampleLength;
  insctrRayleigh = scattering * setting.waveLambdaRayleigh * sampleLength;
}

float ComputeSkyboxChapman(cr::component::skybox::Options setting, float3 eye,
                           float3 V, float3 L, float3 &transmittance,
                           float3 &insctrMie, float3 &insctrRayleigh) {
  bool neg = true;

  float2 outerIntersections = ComputeRaySphereIntersection(
      eye, V, setting.earthCenter, setting.earthAtmTopRadius);
  if (outerIntersections.y < 0.0)
    return 0.0;

  float2 innerIntersections = ComputeRaySphereIntersection(
      eye, V, setting.earthCenter, setting.earthRadius);
  if (innerIntersections.x > 0.0) {
    neg = false;
    outerIntersections.y = innerIntersections.x;
  }

  eye -= setting.earthCenter;

  float3 start = eye + V * max(0.0f, outerIntersections.x);
  float3 end = eye + V * outerIntersections.y;

  AerialPerspective(setting, start, end, V, L, neg, transmittance, insctrMie,
                    insctrRayleigh);

  bool intersectionTest =
      innerIntersections.x < 0.0 && innerIntersections.y < 0.0;
  return intersectionTest ? 1.0 : 0.0;
}

float4 ComputeSkyInscattering(cr::component::skybox::Options setting,
                              float3 eye, float3 V, float3 L) {
  auto insctrMie = float3(0.0);
  auto insctrRayleigh = float3(0.0);
  auto insctrOpticalLength = float3(1.0);
  float intersectionTest = ComputeSkyboxChapman(
      setting, eye, V, L, insctrOpticalLength, insctrMie, insctrRayleigh);

  float phaseTheta = dot(V, L);
  float phaseMie = ComputePhaseMie(phaseTheta, setting.mieG);
  float phaseRayleigh = ComputePhaseRayleigh(phaseTheta);
  float phaseNight = 1.0f - saturate(insctrOpticalLength.x * EPSILON);

  float3 insctrTotalMie = insctrMie * phaseMie;
  float3 insctrTotalRayleigh = insctrRayleigh * phaseRayleigh;

  float3 sky = (insctrTotalMie + insctrTotalRayleigh) * setting.sunRadiance;

  float angle = saturate((1.0f - phaseTheta) * setting.sunRadius);
  float cosAngle = cos(angle * std::numbers::pi_v<float> * 0.5f);
  float edge = ((angle >= 0.9) ? smoothstep(0.9f, 1.0f, angle) : 0.0f);

  float3 limbDarkening = GetTransmittance(setting, -L, V);
  //  limbDarkening *= pow(float3(cosAngle), float3(0.420, 0.503, 0.652)) *
  //  mix(vec3(1.0), float3(1.2,0.9,0.5), edge) * intersectionTest;

  //  sky += limbDarkening;

  return float4(sky, phaseNight * intersectionTest);
}

float noise(float2 uv) {
  const auto a = glm::vec3(uv.x, uv.y, uv.x);
  const auto b = glm::vec3(uv.x, uv.y, uv.y);
  return fract(
      dot(sin(a * b * 1024.0f), float3(341896.483, 891618.637, 602649.7031)));
}
} // namespace

namespace cr {
glm::vec3 sky::at(const glm::vec2 &upside_down_uv, const glm::vec2 &sun_pos) {
  vec2 mouse = _options.sun_pos;
  const auto uv = -upside_down_uv + 1.0f;

  float3 V = ComputeSphereNormal(uv, 0.0, std::numbers::pi_v<float> * 2.0f, 0.0,
                                 std::numbers::pi_v<float>);
  float3 L = ComputeSphereNormal(float2(mouse.x, mouse.y), 0.0,
                                 std::numbers::pi_v<float> * 2.0f, 0.0,
                                 std::numbers::pi_v<float>);

  float3 eye = float3(0, 1000.0, 0);
  float3 sky = ComputeSkyInscattering(_options, eye, V, L);

  return sky;
}

sky::sky(component::skybox::Options options) : _options(options) {}
void sky::use_settings(component::skybox::Options options) {
  _options = options;
}

} // namespace cr
