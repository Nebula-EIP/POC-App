/**
 * @file node_exception.hpp
 * @brief Defines exceptions related to node operations.
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
 * @brief Base exception for errors related to graph operations.
 */
class NodeException : public Exception {
   public:
    using Exception::Exception;
};

// ===================================

/**
 * @brief Thrown when a node cannot be found or is invalid.
 */
class NodeNotFoundException : public NodeException {
   public:
    using NodeException::NodeException;
};

/**
 * @brief Thrown when a connection between nodes or pins is invalid.
 */
class InvalidNodeException : public NodeException {
   public:
    using NodeException::NodeException;
};

/**
 * @brief Thrown when two incompatible data types are used together.
 */
class NodeAlreadyExistsException : public NodeException {
   public:
    using NodeException::NodeException;
};

/**
 * @brief Thrown when a node type is invalid or unsupported.
 */
class NodeTypeException : public NodeException {
   public:
    using NodeException::NodeException;
};

}  // namespace core