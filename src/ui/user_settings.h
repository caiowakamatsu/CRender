#pragma once

class UserSettings
{
public:
    UserSettings() = default;

private:


};

/*
 *
    auto ray = _config.camera.get_ray(
      (static_cast<float>(x) + _rand()) / _config.resolution.x,
      (static_cast<float>(y) + _rand()) / _config.resolution.y);

    auto distance = _scene->cast_ray(ray);

    auto should_bounce  = true;
    auto current_bounce = uint32_t(0);

    auto throughput = glm::vec3(1, 1, 1);
    auto final      = glm::vec3(0, 0, 0);
    while (should_bounce)
    {
        auto intersection = _scene->cast_ray(ray);

        if (intersection.distance == std::numeric_limits<float>::infinity())
        {
            const auto direction = glm::normalize(ray.direction);
            const auto t         = 0.5 * direction.y + 1.0;
            final += throughput *
              glm::vec3(
                       (1.0 - t) * 1.0 + t * 0.5,
                       (1.0 - t) * 1.0 + t * 0.7,
                       (1.0 - t) * 1.0 + t * 1.0);
            break;
        }
        else
        {
            const auto processed_hit   = _process_hit(ray, intersection);
            const auto direct_lighting = _scene->sample_lights_at(intersection.intersection_point);

            final += throughput * processed_hit.emission * (processed_hit.albedo + direct_lighting);
            throughput *= processed_hit.albedo * processed_hit.reflectiveness;

            ray = processed_hit.ray;
        }
        if (current_bounce++ > 10) should_bounce = false;
    }

    // flip Y
    y = _config.resolution.y - 1 - y;

    //    std::lock_guard lock(_buffer_mutex);
    const auto temporal = _current_buffer.get(x, y);
    const auto sample   = glm::min(
      glm::vec3(1, 1, 1),
      (temporal * float(_current_sample_count) + final) / float(_current_sample_count + 1));

    _current_buffer.set(x, y, sample);
 */