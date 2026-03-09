#include "graph_exceptions.hpp"

// GraphException
core::GraphException::GraphException(
    const std::string &err_msg, const std::source_location &location)
    : CoreException(err_msg, location) {}

// NodeNotFoundException
core::NodeNotFoundException::NodeNotFoundException(
    const std::string &err_msg, const std::source_location &location)
    : GraphException(err_msg, location) {}

// InvalidNodeKindException
core::InvalidNodeKindException::InvalidNodeKindException(
    const std::string &err_msg, const std::source_location &location)
    : GraphException(err_msg, location) {}

// DuplicateNodeIdException
core::DuplicateNodeIdException::DuplicateNodeIdException(
    const std::string &err_msg, const std::source_location &location)
    : GraphException(err_msg, location) {}

// CircularDependencyException
core::CircularDependencyException::CircularDependencyException(
    const std::string &err_msg, const std::source_location &location)
    : GraphException(err_msg, location) {}

// GraphModificationException
core::GraphModificationException::GraphModificationException(
    const std::string &err_msg, const std::source_location &location)
    : GraphException(err_msg, location) {}

// EmptyGraphException
core::EmptyGraphException::EmptyGraphException(
    const std::string &err_msg, const std::source_location &location)
    : GraphException(err_msg, location) {}
