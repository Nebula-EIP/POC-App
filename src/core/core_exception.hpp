#pragma once

#include "exception_base.hpp"

namespace core {

class CoreException : utils::BaseException {
 public:
    CoreException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    template <typename... Args>
    CoreException(
        const std::source_location &location,
        std::format_string<Args...> fmt,
        Args &&...args
    );

    ~CoreException() = default;
};

} // namespace core

#include "core_exception.tcc"
