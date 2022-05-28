//
// Created by Caio W on 3/2/22.
//

#ifndef CREBON_TRIANGULAR_SCENE_HPP
#define CREBON_TRIANGULAR_SCENE_HPP

#include <filesystem>
#include <optional>
#include <vector>

#include <scene/ray.hpp>
#include <scene/scene.hpp>

#include <embree3/rtcore_device.h>
#include <embree3/rtcore_geometry.h>
#include <embree3/rtcore_scene.h>

#include <util/logger.hpp>

namespace cr {
class triangular_scene {
private:
  RTCDevice _device = nullptr;
  RTCScene _scene = nullptr;
  RTCGeometry _geom = nullptr;

  std::vector<float> _vertices;
  std::vector<float> _normals;
  std::vector<float> _texcoords;
  std::vector<uint32_t> _indices;
  std::vector<uint32_t> _material_indices;

  std::vector<std::unique_ptr<cr::material>> _materials;

  cr::logger *_logger = nullptr;

  void _load_model(const std::filesystem::path &path);

  void _load_glb(const std::filesystem::path &path);

public:
  explicit triangular_scene(const std::filesystem::path &path, cr::logger *logger);

  ~triangular_scene();

  [[nodiscard]] std::optional<cr::intersection> intersect(const cr::ray &ray);
};
} // namespace cr

#endif // CREBON_TRIANGULAR_SCENE_HPP
