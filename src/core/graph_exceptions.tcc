#pragma once
#include "graph_exceptions.hpp"

// GraphException
template <typename... Args>
core::GraphException::GraphException(const std::source_location &location,
                                     std::format_string<Args...> fmt,
                                     Args &&...args)
    : CoreException(location, fmt, std::forward<Args>(args)...) {}

// NodeNotFoundException
template <typename... Args>
core::NodeNotFoundException::NodeNotFoundException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : GraphException(location, fmt, std::forward<Args>(args)...) {}

// InvalidNodeKindException
template <typename... Args>
core::InvalidNodeKindException::InvalidNodeKindException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : GraphException(location, fmt, std::forward<Args>(args)...) {}

// DuplicateNodeIdException
template <typename... Args>
core::DuplicateNodeIdException::DuplicateNodeIdException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : GraphException(location, fmt, std::forward<Args>(args)...) {}

// CircularDependencyException
template <typename... Args>
core::CircularDependencyException::CircularDependencyException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : GraphException(location, fmt, std::forward<Args>(args)...) {}

// GraphModificationException
template <typename... Args>
core::GraphModificationException::GraphModificationException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : GraphException(location, fmt, std::forward<Args>(args)...) {}

// EmptyGraphException
template <typename... Args>
core::EmptyGraphException::EmptyGraphException(
    const std::source_location &location, std::format_string<Args...> fmt,
    Args &&...args)
    : GraphException(location, fmt, std::forward<Args>(args)...) {}
