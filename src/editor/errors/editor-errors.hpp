#pragma once
#include <stdexcept>
#include <string>
#include <utility>

namespace editor {
// Base exception for the editor module.
class EditorException : public std::runtime_error {
public:
    explicit EditorException(std::string msg)
        : std::runtime_error(std::move(msg)) {}
};
}  // namespace editor
