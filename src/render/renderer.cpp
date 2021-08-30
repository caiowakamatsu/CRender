#include "renderer.h"

namespace
{
    struct processed_hit
    {
        float     emission;
        float     bdxf;
        float     pdf;
        glm::vec3 albedo;
        glm::vec4 colour;
        cr::ray   ray;
    };
    [[nodiscard]] processed_hit
      process_hit(const cr::ray::intersection_record &record, const cr::ray &ray, cr::scene *scene, cr::random *random)
    {
        auto out = processed_hit();

        // Calculate some "required" values
        const auto cos_theta = glm::abs(glm::dot(out.ray.direction, record.normal));

        out.emission = record.material->info.emission;
        if (record.material->info.tex.has_value())
            out.colour = scene->registry()
                           ->entities.get<cr::image>(record.material->info.tex.value())
                           .get_uv(record.uv.x, record.uv.y);
        else
            out.colour = record.material->info.colour;

        out.albedo = glm::vec3(out.colour);

        switch (record.material->info.shade_type)
        {
        case cr::material::glass:
        {
            auto refracted  = glm::vec3();
            auto out_normal = record.normal;
            auto reflected  = glm::reflect(ray.direction, record.normal);

            auto ni_over_nt = 1.0f / record.material->info.ior;

            if (glm::dot(ray.direction, record.normal) > 0)
            {
                out_normal = -record.normal, ni_over_nt = record.material->info.ior;
            }

            const auto uv   = glm::normalize(ray.direction);
            const auto dt   = glm::dot(uv, out_normal);
            const auto disc = 1.0f - ni_over_nt * ni_over_nt * (1 - dt * dt);

            auto refract = false;
            if (disc > 0)
            {
                refracted = ni_over_nt * (uv - out_normal * dt) - out_normal * glm::sqrt(disc);
                refract   = true;
            }

            out.ray.origin = record.intersection_point + out_normal * -0.0001f;
            if (refract)
                out.ray.direction = refracted;
            else
                out.ray.direction = reflected;
        }
        break;
        case cr::material::metal:
        {
            const auto hemp_samp =
              cr::sampling::hemp_cos(record.normal, glm::vec2(random->next_float(), random->next_float()));

            out.ray.origin    = record.intersection_point + record.normal * 0.0001f;
            out.ray.direction = glm::reflect(ray.direction, record.normal);

            out.albedo *= record.material->info.reflectiveness;
            out.bdxf = cr::numbers<float>::inv_pi;
            out.pdf = glm::cos(glm::dot(out.ray.direction, record.normal)) / cr::numbers<float>::pi;
            break;
        }
        case cr::material::smooth:
            auto cos_hemp_dir =
              cr::sampling::hemp_cos(record.normal, glm::vec2(random->next_float(), random->next_float()));

            out.ray.origin    = record.intersection_point + record.normal * 0.0001f;
            out.ray.direction = glm::normalize(cos_hemp_dir);
            out.bdxf          = cr::numbers<float>::inv_pi;
            out.pdf = glm::cos(glm::dot(out.ray.direction, record.normal)) / cr::numbers<float>::pi;
        }

        return out;
    }

}    // namespace

cr::renderer::renderer(
  const uint64_t                    res_x,
  const uint64_t                    res_y,
  const uint64_t                    bounces,
  std::unique_ptr<cr::thread_pool> *pool,
  std::unique_ptr<cr::scene> *      scene)
    : _camera(scene->get()->registry()->camera()), _buffer(res_x, res_y), _normals(res_x, res_y),
      _albedo(res_x, res_y), _depth(res_x, res_y), _res_x(res_x), _res_y(res_y),
      _max_bounces(bounces), _thread_pool(pool), _scene(scene), _raw_buffer(res_x * res_y * 3)
{
    _management_thread = std::thread(
      [this]()
      {
          while (_run_management)
          {
              const auto tasks = _get_tasks();

              if (!tasks.empty() && (_current_sample < _spp_target || _spp_target == 0))
              {
                  _thread_pool->get()->wait_on_tasks(tasks);
                  _current_sample++;
              }
              else
              {
                  if (_current_sample == _spp_target && _current_sample != 0)
                      cr::logger::info(
                        "Finished rendering [{}] samples at resolution [X: {}, Y: {}], took: [{}]s",
                        _spp_target,
                        _res_x,
                        _res_y,
                        _timer.time_since_start());

                  {
                      auto guard = std::unique_lock(_pause_mutex);
                      _pause_cond_var.notify_one();
                  }
                  auto guard = std::unique_lock(_start_mutex);
                  _start_cond_var.wait(guard);
              }
          }
      });
}

cr::renderer::~renderer()
{
    _run_management = false;
    start();    // if we're paused, start it up again
    _management_thread.join();
}

bool cr::renderer::start()
{
    if (_pause)
    {
        _pause = false;
        _buffer.clear();
        _timer.reset();
        for (auto i = 0; i < _res_x * _res_y * 3; i++) _raw_buffer[i] = 0.0f;
        _current_sample = 0;
        _total_rays     = 0;

        auto guard = std::unique_lock(_start_mutex);
        _start_cond_var.notify_all();
        return true;
    }
    return false;
}

bool cr::renderer::pause()
{
    if (!_pause)
    {
        _pause = true;

        auto guard = std::unique_lock(_pause_mutex);
        _pause_cond_var.wait(guard);
        return true;
    }
    return false;
}

void cr::renderer::update(const std::function<void()> &update)
{
    pause();

    update();

    start();
}

void cr::renderer::set_resolution(int x, int y)
{
    _res_x = x;
    _res_y = y;

    _aspect_correction = static_cast<float>(_res_x) / _res_y;

    _buffer  = cr::image(x, y);
    _normals = cr::image(x, y);
    _depth   = cr::image(x, y);
    _albedo  = cr::image(x, y);

    _raw_buffer     = std::vector<float>(x * y * 3);
    _current_sample = 0;
}

void cr::renderer::set_max_bounces(int bounces)
{
    _max_bounces = bounces;
}

void cr::renderer::set_target_spp(uint64_t target)
{
    _spp_target = target;
}

cr::image *cr::renderer::current_progress() noexcept
{
    return &_buffer;
}

cr::image *cr::renderer::current_normals() noexcept
{
    return &_normals;
}

cr::image *cr::renderer::current_albedos() noexcept
{
    return &_albedo;
}

cr::image *cr::renderer::current_depths() noexcept
{
    return &_depth;
}

std::vector<std::function<void()>> cr::renderer::_get_tasks()
{
    auto tasks = std::vector<std::function<void()>>();

    if (_pause) return tasks;

    tasks.reserve(_res_y);

    for (auto y = 0; y < _res_y; y++)
        tasks.emplace_back(
          [this, y]
          {
              auto fired_rays = size_t(0);
              for (auto x = 0; x < _res_x; x++)
              {
                  auto seed = _wang_hash(x * y * (_current_sample + 1) * 384);
                  this->_sample_pixel(x, y, fired_rays, seed);
              }
              _total_rays += fired_rays;
          });

    return tasks;
}

void cr::renderer::_sample_pixel(uint64_t x, uint64_t y, size_t &fired_rays, uint32_t seed)
{
    auto random = cr::random(seed);
    auto ray    = _camera->get_ray(
      (static_cast<float>(x) + random.next_float()) / _res_x,
      (static_cast<float>(y) + random.next_float()) / _res_y,
      _aspect_correction);

    auto throughput = glm::vec3(1.0f, 1.0f, 1.0f);
    auto final      = glm::vec3(0.0f, 0.0f, 0.0f);
    auto albedo     = glm::vec3(0.0f, 0.0f, 0.0f);
    auto normal     = glm::vec3(0.0f, 0.0f, 0.0f);
    auto depth      = 0.0f;

    auto total_bounces = 1;
    for (auto i = 0; i < _max_bounces; i++, total_bounces++)
    {
        auto intersection  = _scene->get()->cast_ray(ray);
        auto processed_hit = ::processed_hit();

        if (intersection.distance == std::numeric_limits<float>::infinity())
        {
            const auto miss_uv = glm::vec2(
              0.5f + atan2f(ray.direction.z, ray.direction.x) / (2 * 3.1415f),
              0.5f - asinf(ray.direction.y) / 3.1415f);

            const auto miss_sample = _scene->get()->sample_skybox(miss_uv.x, miss_uv.y);

            if (i == 0)
            {
                albedo = miss_sample;
                final += throughput * miss_sample;
            }

            break;
        }

        processed_hit = ::process_hit(intersection, ray, _scene->get(), &random);

        // Next event estimation
        // Currently since the scene is small, going through all of the lights is fine
        //            const auto nee = _scene->get()->sample_lights(intersection);

        if (i == 0)
        {
            albedo = processed_hit.albedo;
            normal = intersection.normal;
            depth  = intersection.distance;
        }

        ray = processed_hit.ray;
        throughput *= processed_hit.albedo;
        final += processed_hit.bdxf * throughput * processed_hit.emission / processed_hit.pdf;

        // Light NEE
//        {
//            auto light_nee = _scene->get()->sample_light(intersection, &random);
//            final += throughput * light_nee.contribution / light_nee.pdf;
//        }

        // Sun NEE
        if (_scene->get()->is_sun_enabled())
        {
            auto out_ray = cr::ray(
              intersection.intersection_point + intersection.normal * 0.001f,
              glm::vec3(0.0f));

            auto sample          = cr::sampling::sun::incoming();
            sample.pos           = out_ray.origin;
            sample.normal        = intersection.normal;
            sample.sun_transform = _scene->get()->registry()->sun_transform();
            sample.sun           = _scene->get()->registry()->sun();

            const auto pdf_cos = cr::sampling::sun::sample(sample, &random);
            out_ray.direction  = pdf_cos.dir;

            auto sun_intersection = _scene->get()->cast_ray(out_ray);
            if (sun_intersection.distance == std::numeric_limits<float>::infinity())
                final += throughput * glm::vec3(processed_hit.colour) * pdf_cos.cosine *
                  cr::sampling::sun::sky_colour(
                           out_ray.direction,
                           _scene->get()->registry()->sun()) /
                  pdf_cos.pdf;
        }
    }
    fired_rays += total_bounces;

    // flip Y
    y = _res_y - 1 - y;
    x = _res_x - 1 - x;

    const auto base_index = (x + y * _res_x) * 3;
    _raw_buffer[base_index + 0] += final.x;
    _raw_buffer[base_index + 1] += final.y;
    _raw_buffer[base_index + 2] += final.z;

    _albedo.set(x, y, albedo);
    _normals.set(x, y, normal * .5f + .5f);
    _depth.set(x, y, glm::vec3(glm::min(depth, 200.0f) / 200.f));    // 200.f is the "far" plane.

    _buffer.set(
      x,
      y,
      glm::vec3(
        glm::pow(
          glm::clamp(_raw_buffer[base_index + 0] / float(_current_sample + 1), 0.0f, 1.0f),
          1.f / 2.2f),
        glm::pow(
          glm::clamp(_raw_buffer[base_index + 1] / float(_current_sample + 1), 0.0f, 1.0f),
          1.f / 2.2f),
        glm::pow(
          glm::clamp(_raw_buffer[base_index + 2] / float(_current_sample + 1), 0.0f, 1.0f),
          1.f / 2.2f)));
}

glm::ivec2 cr::renderer::current_resolution() const noexcept
{
    return { _res_x, _res_y };
}

uint64_t cr::renderer::current_sample_count() const noexcept
{
    return _current_sample.load();
}

cr::renderer::renderer_stats cr::renderer::current_stats()
{
    auto stats               = cr::renderer::renderer_stats();
    stats.rays_per_second    = _total_rays / _timer.time_since_start();
    stats.samples_per_second = _current_sample / _timer.time_since_start();
    stats.total_rays         = _total_rays;
    stats.running_time       = _timer.time_since_start();
    return stats;
}

std::uint32_t cr::renderer::_wang_hash(uint32_t val) noexcept
{
    val = (val ^ 61) ^ (val >> 16);
    val *= 9;
    val = val ^ (val >> 4);
    val *= 0x27D4EB2D;
    val = val ^ (val >> 15);
    return val;
}
