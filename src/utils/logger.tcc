#pragma once

#include "logger.hpp"

// Template implementation
template <typename... TArgs>
void utils::Logger::log(LogLevel level, const std::source_location &location,
                        std::string_view formatStr, const TArgs &...args) {
    if (level < minimum_log_level) return;

    std::string message =
        std::vformat(formatStr, std::make_format_args(args...));
    std::string timestamp = get_timestamp();
    auto file = std::filesystem::path(location.file_name()).filename().string();

    std::lock_guard<std::mutex> lock(log_mutex);
    std::cout << timestamp << " [" << to_color(level) << to_string(level)
              << "\033[0m] [" << file << ":" << location.line() << "] "
              << message << std::endl;
}
