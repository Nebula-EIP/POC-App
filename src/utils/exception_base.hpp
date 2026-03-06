#pragma once

#include <exception>
#include <format>
#include <source_location>
#include <sstream>
#include <string>

namespace utils {

class BaseException : public std::exception {
 public:
    BaseException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );
    ~BaseException() = default;

    const char *what() const noexcept override;
    const std::source_location &location() const noexcept;

    /**
     * @brief Returns a detailed error message with source location.
     * Format: "Error: <message>\n  at <function> (<file>:<line>:<column>)"
     */
    std::string GetDetailedMessage() const;

    /**
     * @brief Returns a formatted detailed error message.
     * 
     * Format: "Error: {fmt+args} at {function name} ({file_name}, {line}, {column})"
     * @tparam Args Format argument types
     * @param fmt Format string
     * @param args Format arguments
     * @return Formatted detailed message with source location
     */
    template <typename... Args>
    std::string GetFormattedMessage(std::format_string<Args...> fmt, Args &&...args) const;

 private:
    const std::string what_;
    const std::source_location location_;
};

}  // namespace utils
