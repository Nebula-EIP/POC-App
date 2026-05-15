#pragma once

#include "core_exceptions.hpp"

namespace core {

/**
 * @class GraphException
 * @brief Base exception class for all graph-related errors.
 *
 * This exception is thrown when operations on the graph fail, such as
 * node management, graph state violations, or structural issues.
 */
class GraphException : public CoreException {
   public:
    GraphException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current());

    template <typename... Args>
    GraphException(const std::source_location &location,
                   std::format_string<Args...> fmt, Args &&...args);

    ~GraphException() = default;
};

/**
 * @class NodeNotFoundException
 * @brief Exception thrown when attempting to access a node that doesn't exist.
 *
 * This exception is thrown when trying to retrieve, modify, or remove a node
 * by ID that is not present in the graph.
 */
class NodeNotFoundException : public GraphException {
   public:
    NodeNotFoundException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current());

    template <typename... Args>
    NodeNotFoundException(const std::source_location &location,
                          std::format_string<Args...> fmt, Args &&...args);

    ~NodeNotFoundException() = default;
};

/**
 * @class InvalidNodeKindException
 * @brief Exception thrown when attempting to create a node with an invalid
 * kind.
 *
 * This exception is thrown when trying to create a node with
 * NodeKind::kUndefined or an unsupported node kind.
 */
class InvalidNodeKindException : public GraphException {
   public:
    InvalidNodeKindException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current());

    template <typename... Args>
    InvalidNodeKindException(const std::source_location &location,
                             std::format_string<Args...> fmt, Args &&...args);

    ~InvalidNodeKindException() = default;
};

/**
 * @class DuplicateNodeIdException
 * @brief Exception thrown when attempting to add a node with a duplicate ID.
 *
 * This exception is thrown when trying to insert a node whose ID already
 * exists in the graph.
 */
class DuplicateNodeIdException : public GraphException {
   public:
    DuplicateNodeIdException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current());

    template <typename... Args>
    DuplicateNodeIdException(const std::source_location &location,
                             std::format_string<Args...> fmt, Args &&...args);

    ~DuplicateNodeIdException() = default;
};

/**
 * @class CircularDependencyException
 * @brief Exception thrown when a link would create a circular dependency.
 *
 * This exception is thrown when attempting to create a connection that
 * would result in a cycle in the graph.
 */
class CircularDependencyException : public GraphException {
   public:
    CircularDependencyException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current());

    template <typename... Args>
    CircularDependencyException(const std::source_location &location,
                                std::format_string<Args...> fmt,
                                Args &&...args);

    ~CircularDependencyException() = default;
};

/**
 * @class GraphModificationException
 * @brief Exception thrown when attempting to modify graph in an invalid state.
 *
 * This exception is thrown when trying to modify the graph during operations
 * like traversal or execution where modifications are not allowed.
 */
class GraphModificationException : public GraphException {
   public:
    GraphModificationException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current());

    template <typename... Args>
    GraphModificationException(const std::source_location &location,
                               std::format_string<Args...> fmt, Args &&...args);

    ~GraphModificationException() = default;
};

/**
 * @class EmptyGraphException
 * @brief Exception thrown when operation requires nodes but graph is empty.
 *
 * This exception is thrown when attempting operations that require at least
 * one node but the graph is empty.
 */
class EmptyGraphException : public GraphException {
   public:
    EmptyGraphException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current());

    template <typename... Args>
    EmptyGraphException(const std::source_location &location,
                        std::format_string<Args...> fmt, Args &&...args);

    ~EmptyGraphException() = default;
};

}  // namespace core

#include "graph_exceptions.tcc"
