#include "logger.hpp"

#include <array>
#include <ctime>
#include <iostream>

namespace utils {

const char *ToColor(LogLevel level) {
    switch (level) {
        case LogLevel::kDebug:
            return "\033[0;35m";  // Purple
        case LogLevel::kInfo:
            return "\033[0;36m";  // Cyan
        case LogLevel::kWarning:
            return "\033[0;93m";  // Yellow
        case LogLevel::kError:
            return "\033[1;91m";  // Red
        case LogLevel::kFatal:
            return "\033[1;91m";  // Bright red
    }
    return "\033[0;37m";  // Default white
}

const char *ToString(LogLevel level) {
    switch (level) {
        case LogLevel::kDebug:
            return "DEBUG";
        case LogLevel::kInfo:
            return "INFO";
        case LogLevel::kWarning:
            return "WARNING";
        case LogLevel::kError:
            return "ERROR";
        case LogLevel::kFatal:
            return "FATAL";
    }
    return "UNKNOWN";
}

Logger &Logger::GetInstance() {
    static Logger s_instance;
    return s_instance;
}

void Logger::SetMinimumLogLevel(LogLevel level) { minimum_log_level_ = level; }

std::string Logger::GetTimestamp() const {
    std::time_t now = std::time(nullptr);
    std::array<char, kTimestampBufferSize> buffer{};
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
