#pragma once

#include <exception>
#include <string>

namespace utils {

class BaseException : std::exception {
 public:
    BaseException(const std::string &err_msg);
    ~BaseException() = default;

    const char *what() const noexcept override;
 private:
    const std::string what_;
};

} // namespace utils
