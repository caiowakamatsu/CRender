#pragma once

#include <cstdint>
#include <vector>
#include <thread>
#include <functional>
#include <optional>
#include <atomic>
#include <mutex>
#include <queue>

#include <fmt/core.h>

namespace cr
{
    class thread_pool
    {
    public:
        explicit thread_pool(uint32_t thread_count);

        ~thread_pool();

        void wait_on_tasks(const std::vector<std::function<void()>> &tasks);

    private:
        [[nodiscard]] std::optional<std::function<void()>> _next_task();

        std::atomic<bool> _should_work { true };
        std::atomic<uint32_t> _tasks_submitted = 0;
        std::atomic<uint32_t> _tasks_done = 0;

        std::mutex _work_lock;
        std::mutex _queue_lock;
        std::mutex _finished_lock;

        std::condition_variable _work_conditional;
        std::condition_variable _finished_conditional;

        std::queue<std::function<void()>> _tasks;
        std::vector<std::thread>          _threads;
    };
}    // namespace cr
