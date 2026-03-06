#pragma once
#include <stdexcept>
#include <string>
#include <utility>

namespace core {
// Base exception for the editor module.
class CoreException : public std::runtime_error {
   public:
    explicit CoreException(std::string msg)
        : std::runtime_error(std::move(msg)) {}
};
}  // namespace core
