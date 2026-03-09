#pragma once

#include "core_exceptions.hpp"

namespace core {

/**
 * @class FunctionNodeException
 * @brief Base exception class for all function node-related errors.
 *
 * This exception is thrown when operations on function nodes fail, such as
 * parameter management, return type validation, or function-specific operations.
 */
class FunctionNodeException : public CoreException {
 public:
    FunctionNodeException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    template <typename... Args>
    FunctionNodeException(
        const std::source_location &location,
        std::format_string<Args...> fmt,
        Args &&...args
    );

    ~FunctionNodeException() = default;
};

/**
 * @class InvalidParameterException
 * @brief Exception thrown when parameter operations are invalid.
 *
 * This exception is thrown when attempting invalid parameter operations
 * such as adding duplicate parameter names or invalid parameter indices.
 */
class InvalidParameterException : public FunctionNodeException {
 public:
    InvalidParameterException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    template <typename... Args>
    InvalidParameterException(
        const std::source_location &location,
        std::format_string<Args...> fmt,
        Args &&...args
    );

    ~InvalidParameterException() = default;
};

/**
 * @class ParameterNotFoundException
 * @brief Exception thrown when trying to access a parameter that doesn't exist.
 *
 * This exception is thrown when attempting to remove or access a parameter
 * by name or index that doesn't exist in the function node.
 */
class ParameterNotFoundException : public FunctionNodeException {
 public:
    ParameterNotFoundException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    template <typename... Args>
    ParameterNotFoundException(
        const std::source_location &location,
        std::format_string<Args...> fmt,
        Args &&...args
    );

    ~ParameterNotFoundException() = default;
};

/**
 * @class ConnectedParameterException
 * @brief Exception thrown when attempting to remove a connected parameter.
 *
 * This exception is thrown when trying to remove a parameter that still has
 * active connections. The parameter must be disconnected first before removal.
 */
class ConnectedParameterException : public FunctionNodeException {
 public:
    ConnectedParameterException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    template <typename... Args>
    ConnectedParameterException(
        const std::source_location &location,
        std::format_string<Args...> fmt,
        Args &&...args
    );

    ~ConnectedParameterException() = default;
};

/**
 * @class InvalidReturnTypeException
 * @brief Exception thrown when setting an invalid return type.
 *
 * This exception is thrown when attempting to set an invalid or incompatible
 * return type for a function node.
 */
class InvalidReturnTypeException : public FunctionNodeException {
 public:
    InvalidReturnTypeException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    template <typename... Args>
    InvalidReturnTypeException(
        const std::source_location &location,
        std::format_string<Args...> fmt,
        Args &&...args
    );

    ~InvalidReturnTypeException() = default;
};

}  // namespace core

#include "function_node_exceptions.tcc"
