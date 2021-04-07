#include "scene.h"

void cr::new_scene::add_model(const cr::model &model)
{
    _models.push_back(model);
}

cr::ray::intersection_record cr::new_scene::cast_ray(const cr::ray ray)
{
    auto intersections = std::vector<cr::ray::intersection_record>();
    intersections.reserve(_models.size());

    for (auto &model : _models) intersections.push_back(model.intersect(ray));

    return *std::min_element(
      intersections.begin(),
      intersections.end(),
      [](const cr::ray::intersection_record &lhs, const cr::ray::intersection_record &rhs) {
          return lhs.distance < rhs.distance;
      });
}

// OLD SCENE, DEPRECATED
cr::scene::scene()
{
    _device   = rtcNewDevice(nullptr);
    _scene    = rtcNewScene(_device);
    _geometry = rtcNewGeometry(_device, RTC_GEOMETRY_TYPE_TRIANGLE);
    rtcCommitGeometry(_geometry);
    rtcAttachGeometry(_scene, _geometry);
    rtcCommitScene(_scene);
}

cr::scene::scene(const cr::scene &rhs)
    : _materials(rhs._materials), _material_indices(rhs._material_indices),
      _vertices(rhs._vertices), _indices(rhs._indices)
{
    _device   = rtcNewDevice(nullptr);
    _scene    = rtcNewScene(_device);
    _geometry = rtcNewGeometry(_device, RTC_GEOMETRY_TYPE_TRIANGLE);

    rtcCommitGeometry(_geometry);
    rtcAttachGeometry(_scene, _geometry);
    rtcCommitScene(_scene);

    if (!_vertices.empty()) _rebuild_scene();
}

void cr::scene::add_triangles(
  const std::vector<glm::vec3> &vertices,
  const std::vector<uint64_t> & indices)
{
    _vertices.reserve(_vertices.size() + vertices.size());
    _indices.reserve(indices.size() + indices.size());

    // We need to store this to update the incoming indices
    const auto previous_size = _vertices.size();

    for (const auto &vertex : vertices) _vertices.emplace_back(vertex);
    for (const auto index : indices) _indices.emplace_back(index + previous_size);

    _rebuild_scene();
}

void cr::scene::add_materials(
  const std::vector<cr::material> &materials,
  const std::vector<uint32_t> &    material_indices)
{
    _materials.reserve(_materials.size() + materials.size());
    _indices.reserve(_indices.size() + material_indices.size());

    const auto previous_size = _materials.size();

    for (const auto &material : materials) _materials.emplace_back(material);
    for (const auto index : material_indices) _material_indices.emplace_back(index + previous_size);
}

cr::scene::intersection_record cr::scene::cast_ray(const cr::ray &ray) const noexcept
{
    auto ctx = RTCIntersectContext();
    rtcInitIntersectContext(&ctx);
    auto ray_hit = RTCRayHit();

    ray_hit.ray.org_x = ray.origin.x;
    ray_hit.ray.org_y = ray.origin.y;
    ray_hit.ray.org_z = ray.origin.z;

    ray_hit.ray.dir_x = ray.direction.x;
    ray_hit.ray.dir_y = ray.direction.y;
    ray_hit.ray.dir_z = ray.direction.z;

    ray_hit.ray.tnear  = 0.001f;
    ray_hit.ray.tfar   = std::numeric_limits<float>::infinity();
    ray_hit.ray.mask   = -1;
    ray_hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

    rtcIntersect1(_scene, &ctx, &ray_hit);

    auto record = intersection_record();

    if (ray_hit.hit.geomID == RTC_INVALID_GEOMETRY_ID) return record;

    record.distance           = ray_hit.ray.tfar;
    record.intersection_point = ray.at(ray_hit.ray.tfar);
    record.normal = glm::normalize(glm::vec3(ray_hit.hit.Ng_x, ray_hit.hit.Ng_y, ray_hit.hit.Ng_z));
    record.material = &_materials[_material_indices[ray_hit.hit.primID]];

    return record;
}

glm::vec3 cr::scene::sample_lights_at(const glm::vec3 &point) const noexcept
{
    auto throughput = glm::vec3();

    auto ray   = cr::ray();
    ray.origin = point;

    for (const auto &light : _point_lights)
    {
        ray.direction  = light.sample_dir(point);
        const auto hit = !_ray_occluded(ray);
        if (!hit) throughput += light.colour;
    }

    for (const auto &light : _directional_lights)
    {
        ray.direction  = light.sample_dir(point);
        const auto hit = _ray_occluded(ray);
        if (!hit) throughput += light.colour;
    }

    // Clamp
    throughput = glm::vec3(
      glm::min(throughput.x, 1.0f),
      glm::min(throughput.y, 1.0f),
      glm::min(throughput.z, 1.0f));

    return throughput;
}

std::vector<cr::material> &cr::scene::loaded_materials() noexcept
{
    return _materials;
}

void cr::scene::update_materials(const std::vector<cr::material> &materials)
{
    _materials = materials;
}

void cr::scene::_rebuild_scene()
{
    fmt::print(
      "Rebuilding scene\n\nScene Data\nVertex count: {}\nIndex count: {}\n",
      _vertices.size(),
      _indices.size());

    auto *vertices = static_cast<float *>(rtcSetNewGeometryBuffer(
      _geometry,
      RTC_BUFFER_TYPE_VERTEX,
      0,
      RTC_FORMAT_FLOAT3,
      3 * sizeof(float),
      _vertices.size()));

    auto *indices = static_cast<float *>(rtcSetNewGeometryBuffer(
      _geometry,
      RTC_BUFFER_TYPE_INDEX,
      0,
      RTC_FORMAT_UINT3,
      3 * sizeof(uint32_t),
      _indices.size()));

    if (vertices == nullptr) cr::exit("Couldn't create vertex buffer", 30);

    if (indices == nullptr) cr::exit("Couldn't create index buffer", 31);

    std::memcpy(vertices, _vertices.data(), sizeof(glm::vec3) * _vertices.size());
    std::memcpy(indices, _indices.data(), sizeof(uint32_t) * _indices.size());

    rtcCommitGeometry(_geometry);
    rtcAttachGeometry(_scene, _geometry);
    rtcCommitScene(_scene);
}

bool cr::scene::_ray_occluded(const cr::ray &ray) const noexcept
{
    auto ctx = RTCIntersectContext();
    rtcInitIntersectContext(&ctx);

    auto embree_ray = RTCRay();

    embree_ray.org_x = ray.origin.x;
    embree_ray.org_y = ray.origin.y;
    embree_ray.org_z = ray.origin.z;

    embree_ray.dir_x = ray.direction.x;
    embree_ray.dir_y = ray.direction.y;
    embree_ray.dir_z = ray.direction.z;

    embree_ray.tnear = 0.001f;
    embree_ray.tfar  = std::numeric_limits<float>::infinity();
    embree_ray.mask  = -1;
    embree_ray.flags = 0;

    rtcOccluded1(_scene, &ctx, &embree_ray);

    if (embree_ray.tfar > 0.f) return true;
    return false;
}
