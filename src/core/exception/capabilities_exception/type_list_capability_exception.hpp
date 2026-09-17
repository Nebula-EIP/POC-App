/**
 * @file type_list_capability_exception.hpp
 * @brief Exceptions thrown by the TypeListCapability class
 *
 * @author Created by JeanBizeul
 * @date Created on 17-09-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 17-09-2026
 */

#include "exception/exception.hpp"

namespace core {

/**
 * @brief Thrown when trying to create a type with an already assigned name
 */
class DuplicateTypeNameException : public Exception {
   public:
    using Exception::Exception;
};

}  // namespace core
