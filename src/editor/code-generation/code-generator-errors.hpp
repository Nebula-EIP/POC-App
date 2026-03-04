#pragma once
#include <stdexcept>
#include <string>
#include <utility>

namespace code_generation {

// Base exception for the code_generation module.
class CodeGenerationError : public std::runtime_error {
public:
    explicit CodeGenerationError(std::string msg)
        : std::runtime_error(std::move(msg)) {}
};

// Thrown when an operation is attempted that would cause an overflow.
class IndentationError : public CodeGenerationError {
public:
    explicit IndentationError(std::string were)
        : CodeGenerationError(std::move(were) + ": "
        "Indentation level must be positive") {}
};

// Thrown when an operation is attempted that would cause an overflow.
class CursorOutOfBoundsError : public CodeGenerationError {
public:
    explicit CursorOutOfBoundsError(std::string were)
        : CodeGenerationError(std::move(were) + ": "
        "Cursor position is out of bounds") {}
};

} // namespace code_generation