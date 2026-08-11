/**
 * @file icapability.hpp
 * @brief Base interface for all module capabilities.
 *
 * @author Created by mathys-f
 * @date Created on 06-08-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 06-08-2026
 */

#pragma once

#include <string_view>

#include "../graph/datatypes.hpp"

namespace core {

/**
 * @brief Used to define a type with its corresponding unique id.
 */
struct TypeDefinition {
    DataType id;  ///< Unique id given to each type by the core (ids are uniques
                  ///< across modules)
    std::string_view name;
};

/**
 * @brief Base interface for all module capabilities.
 */
class ICapability {
   public:
    virtual ~ICapability() = default;

    /**
     * @brief Queries the type of capability this instance represents.
     *
     * @return A string view representing the capability type (e.g.,
     * "node_list").
     */
    virtual std::string_view getType() const noexcept = 0;
};

}  // namespace core
