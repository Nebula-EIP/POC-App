#include "exception_base.hpp"

utils::BaseException::BaseException(const std::string &err_msg,
                                    const std::source_location &location)
    : kWhat(err_msg), kLocation(location) {}

const char *utils::BaseException::what() const noexcept {
    return kWhat.c_str();
}

const std::source_location &utils::BaseException::Location() const noexcept {
    return kLocation;
}

std::string utils::BaseException::GetDetailedMessage() const {
    return std::format("Error: {}\n  at {} ({}:{}:{})", kWhat,
                       kLocation.function_name(), kLocation.file_name(),
                       kLocation.line(), kLocation.column());
}

template <typename... Args>
std::string utils::BaseException::GetFormattedMessage(
    std::format_string<Args...> fmt, Args &&...args) const {
    std::string formatted_msg = std::format(fmt, std::forward<Args>(args)...);
    return std::format("Error: {}\n  at {} ({}:{}:{})", formatted_msg,
                       kLocation.function_name(), kLocation.file_name(),
                       kLocation.line(), kLocation.column());
}
