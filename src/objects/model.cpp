#include "model.h"

namespace
{
    [[nodiscard]] cr::ray::intersection_record _intersect(
      const cr::ray &                    ray,
      const cr::entity::embree_ctx &     geometry,
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
        ray_hit.hit.primID = -1;
        ray_hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

        rtcIntersect1(geometry.scene, &ctx, &ray_hit);

        auto record = cr::ray::intersection_record();

        if (ray_hit.hit.geomID == RTC_INVALID_GEOMETRY_ID) return record;

        record.distance           = ray_hit.ray.tfar;
        record.intersection_point = ray.at(ray_hit.ray.tfar);
        record.normal =
          glm::normalize(glm::vec3(ray_hit.hit.Ng_x, ray_hit.hit.Ng_y, ray_hit.hit.Ng_z));
        record.material = &materials.materials[materials.indices[ray_hit.hit.primID]];
        record.prim_id  = ray_hit.hit.primID;

        if (glm::dot(record.normal, ray.direction) > 0) record.normal *= -1;

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
}    // namespace

cr::entity::embree_ctx cr::model::instance_geometry(
  const std::vector<glm::vec3> &vertices,
  const std::vector<uint32_t> & indices,
  const std::vector<glm::vec2> &tex_coords)
{
    auto instance = cr::entity::embree_ctx();
    cr::logger::info("Vertex Count: {}\n", vertices.size());

    rtcSetSharedGeometryBuffer(
      instance.geometry,
      RTC_BUFFER_TYPE_VERTEX,
      0,
      RTC_FORMAT_FLOAT3,
      vertices.data(),
      0,
      sizeof(glm::vec3),
      vertices.size());

    rtcSetSharedGeometryBuffer(
      instance.geometry,
      RTC_BUFFER_TYPE_INDEX,
      0,
      RTC_FORMAT_UINT3,
      indices.data(),
      0,
      3 * sizeof(uint32_t),
      indices.size() / 3);

    rtcSetGeometryVertexAttributeCount(instance.geometry, 1);

    rtcSetSharedGeometryBuffer(
      instance.geometry,
      RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
      0,
      RTC_FORMAT_FLOAT2,
      tex_coords.data(),
      0,
      sizeof(glm::vec2),
      tex_coords.size());

    rtcCommitGeometry(instance.geometry);
    rtcAttachGeometry(instance.scene, instance.geometry);
    rtcCommitScene(instance.scene);

    return instance;
}

cr::ray::intersection_record cr::model::intersect(
  const cr::ray &                    ray,
  const cr::entity::instances &      instances,
  const cr::entity::embree_ctx &     geometry,
  const cr::entity::model_materials &materials)
{
    auto intersection = cr::ray::intersection_record();

    for (const auto &transform : instances.transforms)
    {
        const auto inv         = glm::inverse(transform);
        const auto transformed = cr::ray(
          inv * glm::vec4(ray.origin, 1),
          glm::normalize(inv * glm::vec4(ray.direction, 0)));

        auto current = _intersect(transformed, geometry, materials);

        current.intersection_point =
          glm::vec3(transform * glm::vec4(current.intersection_point, 1.0f));

        if (current.distance != std::numeric_limits<float>::infinity())
            current.distance = glm::distance(current.intersection_point, ray.origin);

        if (current.distance < intersection.distance) intersection = current;
    }

    return intersection;
}
