#include "renderer.h"

namespace
{
    [[nodiscard]] float randf() noexcept
    {
        thread_local std::mt19937                          gen;
        thread_local std::uniform_real_distribution<float> dist(0.f, 1.f);
        return dist(gen);
    }

    struct processed_hit
    {
        float     emission;
        float     reflectiveness;
        glm::vec3 albedo;
        cr::ray   ray;
    };
    [[nodiscard]] processed_hit
      process_hit(const cr::ray::intersection_record &record, const cr::ray &ray)
    {
        auto out = processed_hit();

        out.emission = 0;
        out.albedo   = record.material->colour();

        switch (record.material->mat_type())
        {
        case cr::material::metal:
            out.ray.origin    = record.intersection_point + record.normal * 0.0001f;
            out.ray.direction = glm::reflect(ray.direction, record.normal);

            out.reflectiveness = 0.5;
            break;
        case cr::material::smooth:
            const auto x = ::randf();
            const auto y = ::randf();

            auto cos_hemp_dir = cr::sampling::cos_hemp(x, y);

            if (glm::dot(cos_hemp_dir, record.normal) < 0.0f) cos_hemp_dir *= -1.f;

            out.ray.origin    = record.intersection_point + record.normal * 0.0001f;
            out.ray.direction = glm::normalize(record.normal + cos_hemp_dir);

            out.reflectiveness = std::fmaxf(0.f, glm::dot(record.normal, out.ray.direction));
            break;
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
    : _camera(scene->get()->registry()->camera()), _buffer(res_x, res_y), _res_x(res_x),
      _res_y(res_y), _max_bounces(bounces), _thread_pool(pool), _scene(scene)
{
    _management_thread = std::thread([this]() {
        while (_run_management)
        {
            const auto tasks = _get_tasks();

            if (!tasks.empty())
            {
                _thread_pool->get()->wait_on_tasks(tasks);
                _current_sample++;
            }
            else
            {
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

void cr::renderer::start()
{
    _current_sample = 0;
    auto guard      = std::unique_lock(_start_mutex);
    _start_cond_var.notify_all();
}

void cr::renderer::pause()
{
    _pause = true;

    auto guard = std::unique_lock(_pause_mutex);
    _pause_cond_var.wait(guard);
}

void cr::renderer::update(const std::function<void()> &update)
{
    pause();

    update();

    _buffer.clear();

    start();
}

void cr::renderer::set_resolution(int x, int y)
{
    _res_x = x;
    _res_y = y;

    _aspect_correction = static_cast<float>(_res_x) / static_cast<float>(_res_y);

    _buffer = cr::image(x, y);
}

void cr::renderer::set_max_bounces(int bounces)
{
    _max_bounces = bounces;
}

cr::image *cr::renderer::current_progress() noexcept
{
    return &_buffer;
}

std::vector<std::function<void()>> cr::renderer::_get_tasks()
{
    auto tasks = std::vector<std::function<void()>>();

    if (_pause)
    {
        _pause = false;
        return tasks;
    }

    tasks.reserve(_res_y);

    for (auto y = 0; y < _res_y; y++)
        tasks.emplace_back([this, y] {
            for (auto x = 0; x < _res_x; x++) this->_sample_pixel(x, y);
        });

    return std::move(tasks);
}

void cr::renderer::_sample_pixel(uint64_t x, uint64_t y)
{
    auto ray = cr::camera::get_ray(
      ((static_cast<float>(x) + ::randf()) / _res_x) * _aspect_correction,
      (static_cast<float>(y) + ::randf()) / _res_y,
      *_camera);

    auto throughput = glm::vec3(1.0f, 1.0f, 1.0f);
    auto final      = glm::vec3(0.0f, 0.0f, 0.0f);

    for (auto i = 0; i < _max_bounces; i++)
    {
        auto intersection = _scene->get()->cast_ray(ray);

        if (intersection.distance == std::numeric_limits<float>::infinity())
        {
            const auto miss_uv = glm::vec2(
              0.5f + atan2f(ray.direction.z, ray.direction.x) / (2 * 3.1415f),
              0.5f - asinf(ray.direction.y) / 3.1415f);

            const auto blue  = glm::vec3(0.4, 0.4, 1.0);
            const auto white = glm::vec3(1.0, 1.0, 1.0);
            const auto out   = glm::mix(white, blue, miss_uv.y);
//            final += throughput * out;
            break;
        }
        else
        {
            const auto processed = ::process_hit(intersection, ray);
            const auto light = _scene->get()->sample_lights(intersection.intersection_point, intersection.normal);

            final += throughput * light * processed.albedo;
            throughput *= processed.albedo * processed.reflectiveness;

            ray = processed.ray;
        }
    }
    // flip Y
    y = _res_y - 1 - y;

    const auto temporal = _buffer.get(x, y);
    final = glm::max(glm::vec3(0, 0, 0), glm::min(glm::vec3(1, 1, 1), final));
    const auto sample   = temporal * float(_current_sample) / float(_current_sample + 1) +
      final / float(_current_sample + 1);

    _buffer.set(x, y, sample);
}
