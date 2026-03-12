#pragma once

#include "core_exceptions.hpp"

namespace core {

/**
 * @class ConnectionException
 * @brief Base exception class for all connection and pin-related errors.
 *
 * This exception is thrown when operations involving node connections or
 * pin operations fail, such as invalid pin access or connection validation.
 */
class ConnectionException : public CoreException {
 public:
    ConnectionException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    template <typename... Args>
    ConnectionException(
        const std::source_location &location,
        std::format_string<Args...> fmt,
        Args &&...args
    );

    ~ConnectionException() = default;
};

/**
 * @class InvalidPinIndexException
 * @brief Exception thrown when accessing a pin with an out-of-bounds index.
 *
 * This exception is thrown when trying to access an input or output pin
 * that doesn't exist on the node.
 */
class InvalidPinIndexException : public ConnectionException {
 public:
    InvalidPinIndexException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    template <typename... Args>
    InvalidPinIndexException(
        const std::source_location &location,
        std::format_string<Args...> fmt,
        Args &&...args
    );

    ~InvalidPinIndexException() = default;
};

/**
 * @class PinNotConnectedException
 * @brief Exception thrown when accessing connection data from an unconnected pin.
 *
 * This exception is thrown when trying to retrieve connection information
 * from a pin that has no active connection.
 */
class PinNotConnectedException : public ConnectionException {
 public:
    PinNotConnectedException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    template <typename... Args>
    PinNotConnectedException(
        const std::source_location &location,
        std::format_string<Args...> fmt,
        Args &&...args
    );

    ~PinNotConnectedException() = default;
};

/**
 * @class IncompatiblePinTypesException
 * @brief Exception thrown when attempting to link pins with incompatible types.
 *
 * This exception is thrown when trying to connect an output pin to an input
 * pin with incompatible data types.
 */
class IncompatiblePinTypesException : public ConnectionException {
 public:
    IncompatiblePinTypesException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    template <typename... Args>
    IncompatiblePinTypesException(
        const std::source_location &location,
        std::format_string<Args...> fmt,
        Args &&...args
    );

    ~IncompatiblePinTypesException() = default;
};

/**
 * @class PinAlreadyConnectedException
 * @brief Exception thrown when attempting to connect an already-connected input pin.
 *
 * This exception is thrown when trying to connect an input pin that already
 * has a parent connection (if strict single-parent enforcement is enabled).
 */
class PinAlreadyConnectedException : public ConnectionException {
 public:
    PinAlreadyConnectedException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    template <typename... Args>
    PinAlreadyConnectedException(
        const std::source_location &location,
        std::format_string<Args...> fmt,
        Args &&...args
    );

    ~PinAlreadyConnectedException() = default;
};

/**
 * @class SelfConnectionException
 * @brief Exception thrown when attempting to connect a node to itself.
 *
 * This exception is thrown when trying to create a connection where the
 * source and target nodes are the same.
 */
class SelfConnectionException : public ConnectionException {
 public:
    SelfConnectionException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    template <typename... Args>
    SelfConnectionException(
        const std::source_location &location,
        std::format_string<Args...> fmt,
        Args &&...args
    );

    ~SelfConnectionException() = default;
};

/**
 * @class PinStillConnectedException
 * @brief Exception thrown when attempting to modify pin metadata while pins are still connected.
 *
 * This exception is thrown when a node needs to edit its own pins metadata
 * (such as changing pin count, types, or structure) but one or more pins
 * still have active connections. All pins must be disconnected before
 * modifying the node's pin configuration.
 */
class PinStillConnectedException : public ConnectionException {
 public:
    PinStillConnectedException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current()
    );

    template <typename... Args>
    PinStillConnectedException(
        const std::source_location &location,
        std::format_string<Args...> fmt,
        Args &&...args
    );

    ~PinStillConnectedException() = default;
};

}  // namespace core

#include "connection_exceptions.tcc"
