//
// Created by howto on 29/5/2022.
//

#ifndef CREBON_LOGGER_HPP
#define CREBON_LOGGER_HPP

#include <fmt/format.h>

#include <vector>
#include <mutex>

namespace cr {
class logger {
public:
  enum class level {
    info,
    warning,
    error,
  };

private:
  std::vector<std::pair<level, std::string>> _logs;
  std::mutex _mutex;

public:
  void info(std::string_view fmt, auto &&... args) {
    auto lock = std::lock_guard(_mutex);
    _logs.emplace_back(level::info, fmt::format(fmt, std::forward<decltype(args)>(args)...));
  }

  void warning(std::string_view fmt, auto &&... args) {
    auto lock = std::lock_guard(_mutex);
    _logs.emplace_back(level::warning, fmt::format(fmt, std::forward<decltype(args)>(args)...));
  }

  void error(std::string_view fmt, auto &&... args) {
    auto lock = std::lock_guard(_mutex);
    _logs.emplace_back(level::error, fmt::format(fmt, std::forward<decltype(args)>(args)...));
  }

  [[nodiscard]] std::vector<std::pair<level, std::string>> logs() {
    auto lock = std::lock_guard(_mutex);
    auto logs = _logs;
    _logs.clear();
    return logs;
  }

};
}

#endif // CREBON_LOGGER_HPP
