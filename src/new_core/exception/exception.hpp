/**
 * @file exceptions.hpp
 * @brief Defines the base exception class used throughout the core.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 27-08-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 27-08-2026
 */

#pragma once

#include <stdexcept>

namespace core {

/**
 * @brief Base exception for all core-related errors.
 *
 * All exceptions thrown by the core should derive from this class.
 */
class Exception : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class InternalException : public Exception {
public:
    using Exception::Exception;
};

} // namespace core