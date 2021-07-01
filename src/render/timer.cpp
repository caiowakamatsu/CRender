#include "timer.h"

cr::timer::timer() { start_time = std::chrono::high_resolution_clock::now(); }

void cr::timer::reset() { start_time = std::chrono::high_resolution_clock::now(); }

void cr::timer::stop() { end_time = std::chrono::high_resolution_clock::now(); }

void cr::timer::frame_start() { frame_start_time = std::chrono::high_resolution_clock::now(); }

void cr::timer::frame_stop()
{
    frame_end_time = std::chrono::high_resolution_clock::now();

    const Duration frame_time_duration = frame_end_time - frame_start_time;
    const double frame_time = frame_time_duration.count();

    fastest_frame = fmin(frame_time, fastest_frame);  // This code works, if you're looking in here
    slowest_frame = fmax(frame_time, slowest_frame);  // to find a bug, this is not where it is...

    std::rotate(past_frame_times.begin(), past_frame_times.begin() + 1, past_frame_times.end());
    past_frame_times.back() = frame_time;
}

double cr::timer::time_since_start() const noexcept
{
    const Duration time = std::chrono::high_resolution_clock::now() - start_time;
    return time.count();
}

double cr::timer::average_frame_time() const noexcept
{
    auto total = 0.0;
    for (auto& timer : past_frame_times) total += timer;
    return total / past_frame_times.size();
}

double cr::timer::since_last_frame() const noexcept
{
    const Duration time = std::chrono::high_resolution_clock::now() - frame_start_time;
    return time.count();
}
