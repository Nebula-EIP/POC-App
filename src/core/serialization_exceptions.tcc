#pragma once
#include "serialization_exceptions.hpp"

// SerializationException
template <typename... Args>
core::SerializationException::SerializationException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : CoreException(location, fmt, std::forward<Args>(args)...) {}

// DeserializationException
template <typename... Args>
core::DeserializationException::DeserializationException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : SerializationException(location, fmt, std::forward<Args>(args)...) {}

// InvalidJsonSchemaException
template <typename... Args>
core::InvalidJsonSchemaException::InvalidJsonSchemaException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : SerializationException(location, fmt, std::forward<Args>(args)...) {}

// FileNotFoundException
template <typename... Args>
core::FileNotFoundException::FileNotFoundException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : SerializationException(location, fmt, std::forward<Args>(args)...) {}

// FileAccessException
template <typename... Args>
core::FileAccessException::FileAccessException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : SerializationException(location, fmt, std::forward<Args>(args)...) {}
