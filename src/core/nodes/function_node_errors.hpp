#pragma once
#include <string>

#include "core-errors.hpp"

namespace function_node_errors {
// Base exception for the function_node module.
class FunctionNodeException : public core::CoreException {
   public:
    explicit FunctionNodeException(std::string msg)
        : core::CoreException(std::move(msg)) {}
};

class ParameterNotFoundException : public FunctionNodeException {
   public:
    explicit ParameterNotFoundException(std::string param_name)
        : FunctionNodeException("Parameter not found: " +
                                std::move(param_name)) {}
};

class ParameterNodeCreationException : public FunctionNodeException {
   public:
    explicit ParameterNodeCreationException(std::string msg)
        : FunctionNodeException(std::move(msg)) {}
};

}  // namespace function_node_errors