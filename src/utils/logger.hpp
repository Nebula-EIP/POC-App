#pragma once

#include <filesystem>
#include <format>
#include <mutex>
#include <source_location>
#include <string>
#include <iostream>

/**
 * @brief Logs a debug message with automatic source location capture.
 * @param format Format string (std::format compatible)
 * @param ... Format arguments
 */
#define LOG_DEBUG(format, ...)                                                 \
    utils::Logger::get_instance().log(utils::LogLevel::Debug,                  \
                                       std::source_location::current(),        \
                                       format, ##__VA_ARGS__)

/**
 * @brief Logs an info message with automatic source location capture.
 * @param format Format string (std::format compatible)
 * @param ... Format arguments
 */
#define LOG_INFO(format, ...)                                                  \
    utils::Logger::get_instance().log(utils::LogLevel::Info,                   \
                                       std::source_location::current(),        \
                                       format, ##__VA_ARGS__)

/**
 * @brief Logs a warning message with automatic source location capture.
 * @param format Format string (std::format compatible)
 * @param ... Format arguments
 */
#define LOG_WARNING(format, ...)                                               \
    utils::Logger::get_instance().log(utils::LogLevel::Warning,                \
                                       std::source_location::current(),        \
                                       format, ##__VA_ARGS__)

/**
 * @brief Logs an error message with automatic source location capture.
 * @param format Format string (std::format compatible)
 * @param ... Format arguments
 */
#define LOG_ERROR(format, ...)                                                 \
    utils::Logger::get_instance().log(utils::LogLevel::Error,                  \
                                       std::source_location::current(),        \
                                       format, ##__VA_ARGS__)

/**
 * @brief Logs a fatal message with automatic source location capture.
 * @param format Format string (std::format compatible)
 * @param ... Format arguments
 */
#define LOG_FATAL(format, ...)                                                 \
    utils::Logger::get_instance().log(utils::LogLevel::Fatal,                  \
                                       std::source_location::current(),        \
                                       format, ##__VA_ARGS__)

namespace utils {

/**
 * @enum LogLevel
 * @brief Enumeration of available logging levels.
 */
enum class LogLevel {
    Debug,   ///< Detailed information for debugging
    Info,    ///< General informational messages
    Warning, ///< Warning messages for potentially harmful situations
    Error,   ///< Error messages for serious problems
    Fatal    ///< Fatal error messages for critical failures
};

/**
 * @brief Converts a log level to its ANSI color code.
 * @param level The log level to convert
 * @return ANSI color escape sequence
 */
const char* to_color(LogLevel level);

/**
 * @brief Converts a log level to its string representation.
 * @param level The log level to convert
 * @return String representation of the log level
 */
const char* to_string(LogLevel level);

/**
 * @class Logger
 * @brief Thread-safe singleton logger with formatted output and source location tracking.
 *
 * The Logger class provides a centralized logging system with multiple severity levels,
 * automatic timestamp generation, and source location information. It uses C++20
 * std::format for message formatting and is thread-safe.
 *
 * Example usage:
 * @code
 * LOG_INFO("Application started");
 * LOG_ERROR("Failed to load file: {}", filename);
 * @endcode
 */
class Logger {
  public:
    /**
     * @brief Gets the singleton instance of the Logger.
     * @return Reference to the Logger instance
     */
    static Logger& get_instance();

    /**
     * @brief Sets the minimum log level to display.
     * Messages below this level will be filtered out.
     * @param level The minimum log level
     */
    void set_minimum_log_level(LogLevel level);

    /**
     * @brief Logs a formatted message with the specified level and source location.
     * @tparam TArgs Variadic template arguments for formatting
     * @param level The severity level of the log message
     * @param location Source location where the log was called
     * @param formatStr Format string (std::format compatible)
     * @param args Arguments to format into the message
     */
    template <typename... TArgs>
    void log(LogLevel level, const std::source_location& location,
             std::string_view formatStr, const TArgs&... args);

    // Prevent copying and moving
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

  private:
    /**
     * @brief Private constructor for singleton pattern.
     */
    Logger() = default;

    /**
     * @brief Generates a formatted timestamp string.
     * @return Timestamp in "YYYY-MM-DD HH:MM:SS" format
     */
    std::string get_timestamp() const;

    std::mutex log_mutex;                          ///< Mutex for thread-safe logging
    LogLevel minimum_log_level = LogLevel::Debug;  ///< Minimum level to log
    static constexpr std::size_t k_timestamp_buffer_size = 20; ///< Buffer size for timestamps
};

#include "logger.tcc"

}  // namespace utils
