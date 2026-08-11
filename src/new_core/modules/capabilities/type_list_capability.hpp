/**
 * @file type_list_capability.cpp
 * @brief Interface class for the type list capability
 *
 * @author Created by JeanBizeul
 * @date Created on 11-08-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 11-08-2026
 */

#pragma once

#include <string>
#include <span>
#include <unordered_map>

#include "datatypes.hpp"

namespace core {

namespace capa {

/**
 * @brief List of types the core needs to load and provide to the module owning this capabability.
 */
class ITypeListCapability {
    /**
     * @brief Used only for return values formatting.
     */
    struct TypeDefinition {
        DataType id; ///< Unique id given to each type by the core (ids are uniques across modules)
        std::string_view name;
    };
   public:

    virtual ~ITypeListCapability() = 0;

    /**
     * @brief Used by the core to retreive the types the capability wants to export.
     *
     * @param type_id Available id the capability must assign to one of it's type.
     *
     * @return A pointer to a string_view with the new type, void if all the types of the capability have been assigned.
     */
    virtual const std::string_view *registerType(DataType type_id) const noexcept = 0;

    /**
     * @brief Get a type id from it's name
     *
     * @param type_name Name of a type
     *
     * @return The id of the type
     */
    virtual DataType typeId(std::string_view type_name) const noexcept = 0;

    /**
     * @brief Get a type name from it's id
     *
     * @param type_id Id of a type
     *
     * @return The name of the type
     */
    virtual std::string_view typeName(DataType type_id) const noexcept = 0;

    /**
     * @brief Get the list of all types defined by the capability
     *
     * @return A span of the a struct containing the id & name of the types
     * If the span is empty, it means the capability has not yet registered it's types.
     */
    virtual std::span<const TypeDefinition> types() const noexcept = 0;

   private:
    std::unordered_map<DataType, std::string_view> types;
};

}  // namespace capa

}  // namespace core
