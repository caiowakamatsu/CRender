#include "model.h"

namespace
{
    [[nodiscard]] cr::ray::intersection_record _intersect(
      const cr::ray &                    ray,
      const cr::entity::model_geometry & geometry,
      const cr::entity::model_materials &materials)
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

        ray_hit.ray.tnear  = 0.00001f;
        ray_hit.ray.tfar   = std::numeric_limits<float>::infinity();
        ray_hit.ray.mask   = -1;
        ray_hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

        rtcIntersect1(geometry.scene, &ctx, &ray_hit);

        auto record = cr::ray::intersection_record();

        if (ray_hit.hit.geomID == RTC_INVALID_GEOMETRY_ID) return record;

        record.distance           = ray_hit.ray.tfar;
        record.intersection_point = ray.at(ray_hit.ray.tfar);
        record.normal = glm::normalize(glm::vec3(ray_hit.hit.Ng_x, ray_hit.hit.Ng_y, ray_hit.hit.Ng_z));
        record.material = &materials.materials[materials.indices[ray_hit.hit.primID]];

        rtcInterpolate0(
          geometry.geometry,
          ray_hit.hit.primID,
          ray_hit.hit.u,
          ray_hit.hit.v,
          RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
          0,
          &record.uv.x,
          2);

        return record;
    }
}

cr::entity::model_geometry cr::model::instance_geometry(
  const std::vector<glm::vec3> &vertices,
  const std::vector<uint32_t> & indices,
  const std::vector<glm::vec2> &tex_coords,
  const std::vector<uint32_t> &tex_indices)
{
    auto instance = cr::entity::model_geometry();
    fmt::print("Building model\n\tVertex Count: {}\n", vertices.size());

    auto *vertex_buffer = static_cast<float *>(rtcSetNewGeometryBuffer(
      instance.geometry,
      RTC_BUFFER_TYPE_VERTEX,
      0,
      RTC_FORMAT_FLOAT3,
      3 * sizeof(float),
      vertices.size()));

    auto *index_buffer = static_cast<float *>(rtcSetNewGeometryBuffer(
      instance.geometry,
      RTC_BUFFER_TYPE_INDEX,
      0,
      RTC_FORMAT_UINT3,
      3 * sizeof(uint32_t),
      indices.size()));

    rtcSetGeometryVertexAttributeCount(instance.geometry, 1);

    rtcSetSharedGeometryBuffer(
      instance.geometry,
      RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
      0,
      RTC_FORMAT_FLOAT2,
      tex_coords.data(),
      0,
      sizeof(float) * 2,
      tex_coords.size());

    if (vertex_buffer == nullptr) cr::exit("Couldn't create vertex buffer", 30);

    if (index_buffer == nullptr) cr::exit("Couldn't create index buffer", 31);

    std::memcpy(vertex_buffer, vertices.data(), sizeof(glm::vec3) * vertices.size());
    std::memcpy(index_buffer, indices.data(), sizeof(uint32_t) * indices.size());

    rtcCommitGeometry(instance.geometry);
    rtcAttachGeometry(instance.scene, instance.geometry);
    rtcCommitScene(instance.scene);

    return instance;
}

cr::ray::intersection_record cr::model::intersect(
  const cr::ray &                    ray,
  const cr::entity::transforms &     transforms,
  const cr::entity::model_geometry & geometry,
  const cr::entity::model_materials &materials)
{
    auto intersection = cr::ray::intersection_record();

    for (const auto &transform : transforms.data)
    {
        const auto transformed = cr::ray(transform * glm::vec4(ray.origin, 1), transform * glm::vec4(ray.direction, 0));

        const auto current = _intersect(transformed, geometry, materials);
        if (current.distance < intersection.distance)
            intersection = current;
    }

    return intersection;
}
