#pragma once
#include "connection_exceptions.hpp"

// ConnectionException
template <typename... Args>
core::ConnectionException::ConnectionException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : CoreException(location, fmt, std::forward<Args>(args)...) {}

// InvalidPinIndexException
template <typename... Args>
core::InvalidPinIndexException::InvalidPinIndexException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : ConnectionException(location, fmt, std::forward<Args>(args)...) {}

// PinNotConnectedException
template <typename... Args>
core::PinNotConnectedException::PinNotConnectedException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : ConnectionException(location, fmt, std::forward<Args>(args)...) {}

// IncompatiblePinTypesException
template <typename... Args>
core::IncompatiblePinTypesException::IncompatiblePinTypesException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : ConnectionException(location, fmt, std::forward<Args>(args)...) {}

// PinAlreadyConnectedException
template <typename... Args>
core::PinAlreadyConnectedException::PinAlreadyConnectedException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : ConnectionException(location, fmt, std::forward<Args>(args)...) {}

// SelfConnectionException
template <typename... Args>
core::SelfConnectionException::SelfConnectionException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : ConnectionException(location, fmt, std::forward<Args>(args)...) {}
