#include "model.h"

cr::model::model(
  const std::vector<glm::vec3> &   vertices,
  const std::vector<cr::material> &materials,
  const std::vector<uint32_t>      vertex_indices,
  const std::vector<uint32_t>      material_indices)
{
    _materials.data    = materials;
    _materials.indices = material_indices;

    fmt::print(
      "Building model\n\tVertex Count: {}\n\tMaterial Count: {}",
      vertices.size(),
      materials.size());

    auto *vertex_buffer = static_cast<float *>(rtcSetNewGeometryBuffer(
      _geometry,
      RTC_BUFFER_TYPE_VERTEX,
      0,
      RTC_FORMAT_FLOAT3,
      3 * sizeof(float),
      vertices.size()));

    auto *index_buffer = static_cast<float *>(rtcSetNewGeometryBuffer(
      _geometry,
      RTC_BUFFER_TYPE_INDEX,
      0,
      RTC_FORMAT_UINT3,
      3 * sizeof(uint32_t),
      vertex_indices.size()));

    if (vertex_buffer == nullptr) cr::exit("Couldn't create vertex buffer", 30);

    if (index_buffer == nullptr) cr::exit("Couldn't create index buffer", 31);

    std::memcpy(vertex_buffer, vertices.data(), sizeof(glm::vec3) * vertices.size());
    std::memcpy(index_buffer, vertex_indices.data(), sizeof(uint32_t) * vertex_indices.size());

    rtcCommitGeometry(_geometry);
    rtcAttachGeometry(_scene, _geometry);
    rtcCommitScene(_scene);
}

cr::ray::intersection_record cr::model::intersect(const cr::ray &ray)
{
    auto records = std::vector<cr::ray::intersection_record>();
    records.reserve(_transforms.size());

    for (const auto &transform : _transforms)
    {
        auto transformed = ray;

        transformed.origin    = transform * glm::vec4(ray.origin, 0.0f);
        transformed.direction = transform * glm::vec4(ray.direction, 1.0f);

        records.push_back(_intersect_self(transformed));
    }

    return *std::min_element(
      records.begin(),
      records.end(),
      [](const cr::ray::intersection_record &lhs, const cr::ray::intersection_record &rhs) {
          return lhs.distance < rhs.distance;
      });
}

void cr::model::add_transform(const glm::mat4 &transform)
{
    _transforms.push_back(transform);
}

cr::ray::intersection_record cr::model::_intersect_self(const cr::ray &ray)
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

    auto record = cr::ray::intersection_record();

    if (ray_hit.hit.geomID == RTC_INVALID_GEOMETRY_ID) return record;

    record.distance           = ray_hit.ray.tfar;
    record.intersection_point = ray.at(ray_hit.ray.tfar);
    record.normal = glm::normalize(glm::vec3(ray_hit.hit.Ng_x, ray_hit.hit.Ng_y, ray_hit.hit.Ng_z));
    record.material = &_materials.data[_materials.indices[ray_hit.hit.primID]];

    return record;
}
