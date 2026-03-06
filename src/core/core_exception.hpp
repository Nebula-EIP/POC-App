#pragma once

#include "exception_base.hpp"

namespace core {

class CoreException : utils::BaseException {
 public:
    CoreException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    ~CoreException() = default;
};

} // namespace core
