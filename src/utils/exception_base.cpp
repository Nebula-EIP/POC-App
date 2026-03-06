#include "exception_base.hpp"

utils::BaseException::BaseException(const std::string &err_msg)
    : what_(err_msg) {}

const char *utils::BaseException::what() const noexcept {
    return what_.c_str();
}
