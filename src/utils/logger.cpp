#include "logger.hpp"

#include <array>
#include <ctime>
#include <iostream>

namespace utils {

const char* to_color(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "\033[0;35m";  // Purple
        case LogLevel::Info:
            return "\033[0;36m";  // Cyan
        case LogLevel::Warning:
            return "\033[0;93m";  // Yellow
        case LogLevel::Error:
            return "\033[1;91m";  // Red
        case LogLevel::Fatal:
            return "\033[1;91m";  // Bright red
    }
    return "\033[0;37m";  // Default white
}

const char* to_string(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARNING";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Fatal:
            return "FATAL";
    }
    return "UNKNOWN";
}

Logger& Logger::get_instance() {
    static Logger s_instance;
    return s_instance;
}

void Logger::set_minimum_log_level(LogLevel level) {
    minimum_log_level = level;
}

std::string Logger::get_timestamp() const {
    std::time_t now = std::time(nullptr);
    std::array<char, k_timestamp_buffer_size> buffer{};
    std::tm tm{};

#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &now);  // Windows: thread-safe
#else
    localtime_r(&now, &tm);  // Linux/Unix: thread-safe
#endif

    std::strftime(buffer.data(), buffer.size(), "%F %T", &tm);
    return std::string(buffer.data());
}

}  // namespace utils
