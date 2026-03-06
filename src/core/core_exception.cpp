#include "core_exception.hpp"

core::CoreException::CoreException(
    const std::string &err_msg, const std::source_location &location)
    : utils::BaseException(err_msg, location) {}
