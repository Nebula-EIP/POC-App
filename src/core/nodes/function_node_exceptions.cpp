#include "function_node_exceptions.hpp"

// FunctionNodeException
core::FunctionNodeException::FunctionNodeException(
    const std::string &err_msg, const std::source_location &location)
    : CoreException(err_msg, location) {}

// InvalidParameterException
core::InvalidParameterException::InvalidParameterException(
    const std::string &err_msg, const std::source_location &location)
    : FunctionNodeException(err_msg, location) {}

// ParameterNotFoundException
core::ParameterNotFoundException::ParameterNotFoundException(
    const std::string &err_msg, const std::source_location &location)
    : FunctionNodeException(err_msg, location) {}

// ConnectedParameterException
core::ConnectedParameterException::ConnectedParameterException(
    const std::string &err_msg, const std::source_location &location)
    : FunctionNodeException(err_msg, location) {}

// InvalidReturnTypeException
core::InvalidReturnTypeException::InvalidReturnTypeException(
    const std::string &err_msg, const std::source_location &location)
    : FunctionNodeException(err_msg, location) {}
