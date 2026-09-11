#pragma once

#include "logger.hpp"

// Template implementation
template <typename... TArgs>
void utils::Logger::Log(LogLevel level, const std::source_location &location,
                        std::string_view formatStr, const TArgs &...args) {
    if (level < minimum_log_level_) return;

    std::string message =
        std::vformat(formatStr, std::make_format_args(args...));
    std::string timestamp = GetTimestamp();
    auto file = std::filesystem::path(location.file_name()).filename().string();

    std::lock_guard<std::mutex> lock(log_mutex_);
    std::cout << timestamp << " [" << ToColor(level) << ToString(level)
              << "\033[0m] [" << file << ":" << location.line() << "] "
              << message << std::endl;
}
