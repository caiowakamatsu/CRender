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

        out.emission = record.material->_emission;
        out.albedo   = record.material->colour();

        switch (record.material->mat_type())
        {
        case cr::material::metal:
            out.ray.origin    = record.intersection_point;
            out.ray.direction = glm::reflect(ray.direction, record.normal);

            out.reflectiveness = 0.5;
            break;
        case cr::material::smooth:
            const auto x = ::randf();
            const auto y = ::randf();

            auto cos_hemp_dir = cr::sampling::cos_hemp(x, y);

            if (glm::dot(cos_hemp_dir, record.normal) < 0.0f) cos_hemp_dir *= -1.f;

            out.ray.origin    = record.intersection_point;
            out.ray.direction = glm::normalize(record.normal + cos_hemp_dir);

            out.reflectiveness = std::fmaxf(0.f, glm::dot(record.normal, out.ray.direction));
            break;
        }

        return out;
    }
}    // namespace

cr::new_renderer::new_renderer(
  const cr::camera &camera,
  const uint64_t    res_x,
  const uint64_t    res_y,
  cr::thread_pool * pool,
  cr::new_scene *   scene)
    : _camera(camera), _buffer(res_x, res_y), _res_x(res_x), _res_y(res_y), thread_pool(pool),
      scene(scene)
{
    _management_thread = std::thread([this]() {
        while (_run_management)
        {
            const auto tasks = _get_tasks();

            if (tasks.empty())
            {
                auto guard = std::unique_lock(_start_mutex);
                _start_cond_var.wait(guard);
            }
            else
                thread_pool->wait_on_tasks(tasks);
        }
    });
}

cr::new_renderer::~new_renderer()
{
    _run_management = false;
    start(); // if we're paused, start it up again
    _management_thread.join();
}

void cr::new_renderer::start()
{
    auto guard = std::unique_lock(_start_mutex);
    _start_cond_var.notify_all();
}

void cr::new_renderer::pause()
{
    _pause = true;
}

cr::image *cr::new_renderer::current_progress() noexcept
{
    return &_buffer;
}

std::vector<std::function<void()>> cr::new_renderer::_get_tasks()
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

void cr::new_renderer::_sample_pixel(uint64_t x, uint64_t y)
{
    auto ray = _camera.get_ray(
      (static_cast<float>(x) + ::randf()) / _res_x,
      (static_cast<float>(y) + ::randf()) / _res_y);

    auto throughput = glm::vec3(1.0f, 1.0f, 1.0f);
    auto final      = glm::vec3(0.0f, 0.0f, 0.0f);

    for (auto i = 0; i < 8; i++)
    {
        auto intersection = scene->cast_ray(ray);

        if (intersection.distance == std::numeric_limits<float>::infinity())
        {
            const auto direction = glm::normalize(ray.direction);
            const auto t         = 0.5 * direction.y + 1.0f;
            final += throughput *
              glm::vec3(
                       (1.0 - t) * 1.0 + t * 0.5,
                       (1.0 - t) * 1.0 + t * 0.7,
                       (1.0 - t) * 1.0 + t * 1.0);
        }
        else
        {
            const auto processed = ::process_hit(intersection, ray);

            final += throughput * processed.emission * processed.albedo;
            throughput *= processed.albedo * processed.reflectiveness;

            ray = processed.ray;
        }
    }
    // flip Y
    y = _res_y - 1 - y;

    const auto temporal = _buffer.get(x, y);
    const auto sample   = glm::min(
      glm::vec3(1, 1, 1),
      (temporal * float(_current_sample) + final) / float(_current_sample + 1));

    _buffer.set(x, y, sample);
}

// OLD DEPRECATED

cr::renderer::renderer(cr::renderer::config config) : _thread_pool(config.thread_count)
{
    _config = config;

    _current_buffer = cr::image(_config.resolution.x, _config.resolution.y);
}

cr::renderer::~renderer()
{
    {
        auto lock          = std::lock_guard(_should_render_mutex);
        _should_render_val = false;
    }
    _render_thread.join();
}

void cr::renderer::start()
{
    _render_thread = std::thread([this] {
        const auto tasks = _get_tasks();

        while (_should_render())
        {
            _thread_pool.wait_on_tasks(tasks);
            _current_sample_count++;
        }
    });
}

std::vector<std::function<void()>> cr::renderer::_get_tasks()
{
    auto tasks = std::vector<std::function<void()>>();

    switch (_config.render_mode)
    {
    case config::LINE:
        tasks.reserve(_config.resolution.y);
        for (uint64_t y = 0; y < _config.resolution.y; y++)
        {
            tasks.emplace_back([this, y] {
                for (uint64_t x = 0; x < _config.resolution.x; x++) _sample_pixel(x, y);
            });
        }
        break;
    case config::TILE: cr::exit("Not implemented"); break;
    }

    return std::move(tasks);
}

void cr::renderer::_sample_pixel(uint64_t x, uint64_t y)
{
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
}

cr::image cr::renderer::current_progress()
{
    std::lock_guard lock(_buffer_mutex);
    return _current_buffer;
}

cr::renderer::_processed_hit cr::renderer::_process_hit(
  const cr::ray &                       ray,
  const cr::scene::intersection_record &record) const noexcept
{
    auto out = _processed_hit();

    const auto material = record.material;

    switch (material->mat_type())
    {
    case material::metal:
    {
        /*
         * Cook-Torrance BRDF
         *
         * v = Inverse Ray Direction
         * l = Scattered Ray Direction
         * n = Surface Normal
         * h = Half Unit Vector Between l and v
         * a = roughness
         *
         *           D(h,a) G(v,l,a) F(v,h,f0))
         * Fr(v,l) = --------------------------
         *                4(n * v) (n * l)
         */

        const auto d_term = cr::sampling::cook_torrence::specular_d(
          glm::dot(record.normal, ray.inv_dir()),
          material->_roughness);

        const auto g_term = cr::sampling::cook_torrence::specular_g(
          std::max(0.f, glm::dot(record.normal, ray.direction)),
          glm::dot(record.normal, -ray.direction),
          material->_roughness);

        //        const auto f_term =
        //          cr::sampling::cook_torrence::specular_f(material->_ior, glm::dot(ray.direction,
        //          ))

        out.emission       = material->_emission;
        out.ray.origin     = record.intersection_point;
        out.ray.direction  = glm::reflect(ray.direction, record.normal);
        out.reflectiveness = 0.5;
        out.albedo         = material->colour();
    }
    break;
    case material::smooth:
    {
        // Cosine Hemp Sampling
        const auto x = _rand();
        const auto y = _rand();

        auto cos_hemp_dir = cr::sampling::cos_hemp(x, y);

        if (glm::dot(cos_hemp_dir, record.normal) < 0.0f) cos_hemp_dir *= -1.f;

        out.ray.origin    = record.intersection_point;
        out.ray.direction = glm::normalize(record.normal + cos_hemp_dir);

        out.reflectiveness = std::fmaxf(0.f, glm::dot(record.normal, out.ray.direction));
        out.albedo         = material->colour();
        out.emission       = material->_emission;
    }
    break;
    }

    return out;
}

float cr::renderer::_rand() noexcept
{
    thread_local static std::mt19937                          gen;
    thread_local static std::uniform_real_distribution<float> dist(0.f, 1.f);
    return dist(gen);
}

bool cr::renderer::_should_render()
{
    auto       lock    = std::lock_guard(_should_render_mutex);
    const auto _should = _should_render_val;
    return _should;
}

std::unique_ptr<cr::renderer> cr::renderer::from_config(cr::renderer::config config)
{
    // Essentially create a clone of our current one
    auto renderer = std::make_unique<cr::renderer>(config);

    renderer->_scene = std::make_unique<cr::scene>(*_scene);

    return std::move(renderer);
}

void cr::renderer::attach_scene(std::unique_ptr<cr::scene> scene)
{
    _scene = std::move(scene);
}

void cr::renderer::stop()
{
    {
        auto lock          = std::lock_guard(_should_render_mutex);
        _should_render_val = false;
    }
    _thread_pool.clear_tasks();
}

cr::scene *cr::renderer::scene() const noexcept
{
    return _scene.get();
}
cr::renderer::config cr::renderer::current_config() const noexcept
{
    return _config;
}
