#pragma once
#include <string>

#include "errors/editor-errors.hpp"

namespace code_generation {

// Base exception for the code_generation module.
class CodeGenerationError : public editor::EditorException {
   public:
    explicit CodeGenerationError(std::string msg)
        : editor::EditorException(std::move(msg)) {}
};

// Thrown when an operation is attempted that would cause an overflow.
class IndentationError : public CodeGenerationError {
   public:
    explicit IndentationError(std::string were)
        : CodeGenerationError(std::move(were) +
                              ": "
                              "Indentation level must be positive") {}

    IndentationError(std::string were, int level)
        : CodeGenerationError(std::move(were) +
                              ": "
                              "Indentation level must be positive (level=" +
                              std::to_string(level) + ")") {}
};

// Thrown when an operation is attempted that would cause an overflow.
class CursorOutOfBoundsError : public CodeGenerationError {
   public:
    explicit CursorOutOfBoundsError(std::string were)
        : CodeGenerationError(std::move(were) +
                              ": "
                              "Cursor position is out of bounds") {}

    CursorOutOfBoundsError(std::string were, int position,
                           long long contentLength)
        : CodeGenerationError(std::move(were) +
                              ": "
                              "Cursor position is out of bounds (pos=" +
                              std::to_string(position) + ", length=" +
                              std::to_string(contentLength) + ")") {}
};

}  // namespace code_generation