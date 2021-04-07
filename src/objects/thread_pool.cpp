#include "thread_pool.h"

cr::thread_pool::thread_pool(uint32_t thread_count)
{
    _threads.reserve(thread_count);
    for (auto i = 0; i < thread_count; i++)
    {
        _threads.emplace_back([this] {
            while (_should_work)
            {
                {
                    std::unique_lock lock(_work_lock);
                    _work_conditional.wait(lock);
                }
                auto task = _next_task();
                while (task.has_value())
                {
                    task.value()();
                    task = _next_task();
                }
            }
        });
    }
}

cr::thread_pool::~thread_pool()
{
    _should_work = false;
    {
        std::lock_guard lock(_queue_lock);
        while (!_tasks.empty()) _tasks.pop();
    }
    {
        std::lock_guard lock(_work_lock);
        _work_conditional.notify_all();
    }
    for (auto &thread : _threads) thread.join();
}

void cr::thread_pool::wait_on_tasks(const std::vector<std::function<void()>> &tasks)
{
    _tasks_submitted = tasks.size();
    {
        std::lock_guard lock(_queue_lock);
        for (const auto &task : tasks)
            _tasks.push([task, this]() {
                task();
                if (_tasks_submitted <= ++_tasks_done)
                    _finished_conditional.notify_all();
            });
    }
    _work_conditional.notify_all();

    {
        std::unique_lock lock(_finished_lock);
        _finished_conditional.wait(lock);
        _tasks_submitted = 0;
        _tasks_done      = 0;
    }
}

std::optional<std::function<void()>> cr::thread_pool::_next_task()
{
    std::lock_guard guard(_queue_lock);

    while (!_tasks.empty())
    {
        auto val = _tasks.front();
        _tasks.pop();
        return val;
    }

    return {};
}
