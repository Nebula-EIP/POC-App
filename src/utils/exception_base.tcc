#pragma once
#include "exception_base.hpp"

template <typename... Args>
utils::BaseException::BaseException(const std::source_location &location,
                                    std::format_string<Args...> fmt,
                                    Args &&...args) {}
