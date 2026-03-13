#pragma once
#include "core_exceptions.hpp"

template <typename... Args>
core::CoreException::CoreException(const std::source_location &location,
                                   std::format_string<Args...> fmt,
                                   Args &&...args)
    : BaseException(location, fmt, std::forward<Args>(args)...) {}
