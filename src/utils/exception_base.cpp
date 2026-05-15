#include "exception_base.hpp"

utils::BaseException::BaseException(const std::string &err_msg,
                                    const std::source_location &location)
    : what_(err_msg), location_(location) {}

const char *utils::BaseException::what() const noexcept {
    return what_.c_str();
}

const std::source_location &utils::BaseException::location() const noexcept {
    return location_;
}

std::string utils::BaseException::GetDetailedMessage() const {
    return std::format("Error: {}\n  at {} ({}:{}:{})", what_,
                       location_.function_name(), location_.file_name(),
                       location_.line(), location_.column());
}

template <typename... Args>
std::string utils::BaseException::GetFormattedMessage(
    std::format_string<Args...> fmt, Args &&...args) const {
    std::string formatted_msg = std::format(fmt, std::forward<Args>(args)...);
    return std::format("Error: {}\n  at {} ({}:{}:{})", formatted_msg,
                       location_.function_name(), location_.file_name(),
                       location_.line(), location_.column());
}
