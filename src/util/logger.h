#pragma once

#include <fmt/core.h>
#include <string_view>

namespace cr::logger
{
    void initialize();

    void cleanup();

    [[nodiscard]] void read_messages(std::vector<std::string> &into);

    void error(const std::string_view message);

    template<typename... Args>
    void error(const std::string_view message, Args &&... args)
    {
        error(fmt::format(message, std::forward<Args>(args)...));
    }

    void warn(const std::string_view message);

    template<typename... Args>
    void warn(const std::string_view message, Args &&... args)
    {
        warn(fmt::format(message, std::forward<Args>(args)...));
    }

    void info(const std::string_view message);

    template<typename... Args>
    void info(const std::string_view message, Args &&... args)
    {
        info(fmt::format(message, std::forward<Args>(args)...));
    }

    void debug(const std::string_view message);

    template<typename... Args>
    void debug(const std::string_view message, Args &&... args)
    {
        debug(fmt::format(message, std::forward<Args>(args)...));
    }
}    // namespace Logger
