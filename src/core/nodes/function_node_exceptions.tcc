#pragma once
#include "function_node_exceptions.hpp"

// FunctionNodeException
template <typename... Args>
core::FunctionNodeException::FunctionNodeException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : CoreException(location, fmt, std::forward<Args>(args)...) {}

// InvalidParameterException
template <typename... Args>
core::InvalidParameterException::InvalidParameterException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : FunctionNodeException(location, fmt, std::forward<Args>(args)...) {}

// ParameterNotFoundException
template <typename... Args>
core::ParameterNotFoundException::ParameterNotFoundException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : FunctionNodeException(location, fmt, std::forward<Args>(args)...) {}

// ConnectedParameterException
template <typename... Args>
core::ConnectedParameterException::ConnectedParameterException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : FunctionNodeException(location, fmt, std::forward<Args>(args)...) {}

// InvalidReturnTypeException
template <typename... Args>
core::InvalidReturnTypeException::InvalidReturnTypeException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : FunctionNodeException(location, fmt, std::forward<Args>(args)...) {}
