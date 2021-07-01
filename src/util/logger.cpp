#include "logger.h"

#include <fstream>
#include <fmt/chrono.h>
#include <fmt/color.h>

#include <filesystem>

namespace
{
    constexpr fmt::color ERROR_COLOR   = fmt::color::red;
    constexpr fmt::color WARNING_COLOR = fmt::color::yellow;
    constexpr fmt::color INFO_COLOR    = fmt::color::white;
    constexpr fmt::color DEBUG_COLOR   = fmt::color::blue_violet;

    constexpr bool LOG_ERRORS   = true;
    constexpr bool LOG_WARNINGS = true;
    constexpr bool LOG_INFO     = true;
    constexpr bool LOG_DEBUG    = true;

    std::string message_format;
    std::vector<std::string> messages;

}    // namespace

void cr::logger::initialize()
{
    // setup_console();

    message_format = "";

    // Create the logs directory if it doesn't exist
    if (!std::filesystem::is_directory("logs") || !std::filesystem::exists("logs"))
    {
        std::filesystem::create_directory("logs");
    }

    const auto time = fmt::localtime(std::time(nullptr));

    std::string file_format = "logs/{y:0>4}-{m:0>2}-{d:0>2}_{hh:0>2}-{mm:0>2}-{ss:0>2}.txt";

    std::string filename = fmt::format(
      file_format,
      fmt::arg("y", time.tm_year + 1900),
      fmt::arg("m", time.tm_mon),
      fmt::arg("d", time.tm_mday),
      fmt::arg("hh", time.tm_hour),
      fmt::arg("mm", time.tm_min),
      fmt::arg("ss", time.tm_sec));
}

void cr::logger::cleanup()
{
    // restore_console();
}

static void log(const std::string_view message, const std::string_view type, fmt::color color)
{
    // TODO do all of this on some other thread!

    const auto time = fmt::localtime(std::time(nullptr));

    auto parsed_raw_msg = fmt::format(
    "[{hh:0>2}:{mm:0>2}:{ss:0>2}] [{type}]: {msg}",
      fmt::arg("msg", message),
      fmt::arg("type", type),
      fmt::arg("y", time.tm_year),
      fmt::arg("m", time.tm_mon),
      fmt::arg("d", time.tm_mday),
      fmt::arg("hh", time.tm_hour),
      fmt::arg("mm", time.tm_min),
      fmt::arg("ss", time.tm_sec));

    ::messages.push_back(std::move(parsed_raw_msg));
}

void cr::logger::error(const std::string_view message)
{
    if constexpr (LOG_ERRORS) log(message, "ERROR", ERROR_COLOR);
}

void cr::logger::warn(const std::string_view message)
{
    if constexpr (LOG_WARNINGS) log(message, "WARN", WARNING_COLOR);
}

void cr::logger::info(const std::string_view message)
{
    if constexpr (LOG_INFO) log(message, "INFO", INFO_COLOR);
}

void cr::logger::read_messages(std::vector<std::string> &into)
{
    into.reserve(::messages.size() + into.size());
    for (auto &str : ::messages)
        into.push_back(std::move(str));
    ::messages.clear();
}
