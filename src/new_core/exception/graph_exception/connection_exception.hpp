/**
 * @file connection_exception.hpp
 * @brief Defines exceptions related to connection operations.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 27-08-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 27-08-2026
 */

#pragma once

#include "exception/exception.hpp"

namespace core {

/**
 * @brief Base exception for errors related to connection operations.
 */
class ConnectionException : public Exception {
public:
    using Exception::Exception;
};

// ========================================

/**
 * @brief Thrown when a connection between nodes or pins is invalid.
 */
class InvalidConnectionException : public ConnectionException {
public:
    using ConnectionException::ConnectionException;
};

/**
 * @brief Thrown when a connection between nodes or pins already exists.
 */
class ConnectionAlreadyExistsException : public ConnectionException {
public:
    using ConnectionException::ConnectionException;
};

/**
 * @brief Thrown when a connection between nodes or pins cannot be found.
 */
class ConnectionNotFoundException : public ConnectionException {
public:
    using ConnectionException::ConnectionException;
};

/**
 * @brief Thrown when two incompatible data types are used together in a
 * connection.
 */
class TypeMismatchException : public ConnectionException {
public:
    using ConnectionException::ConnectionException;
};

} // namespace core