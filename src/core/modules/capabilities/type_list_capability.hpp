/**
 * @file type_list_capability.hpp
 * @brief Interface class for the type list capability
 *
 * @author Created by JeanBizeul
 * @date Created on 11-08-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 07-09-2026
 */

#pragma once

#include <list>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../../graph/datatypes.hpp"
#include "../icapability.hpp"

namespace core {
namespace capa {

/**
 * @brief List of types the core needs to load and provide to the module owning
 * this capabability.
 */
class ITypeListCapability : public core::ICapability {
   public:
    /**
     * @brief Used only for return values formatting.
     */
    struct TypeDefinition {
        DataType id_;  ///< Unique id given to each type by the core (ids are
                       ///< uniques across modules)
        std::string_view name_;
    };

    virtual ~ITypeListCapability() = default;

    /**
     * @brief Used by the core to retreive the types the capability wants to
     * export.
     *
     * @param type_id Available id the capability must assign to one of it's
     * type.
     *
     * @return A pointer to a string_view with the new type, nullptr if all the
     * types of the capability have been assigned.
     */
    virtual const std::string_view *RegisterType(
        DataType type_id) const noexcept = 0;

    /**
     * @brief Get a type id from it's name
     *
     * @param type_name Name of a type
     *
     * @return The id of the type
     */
    virtual DataType TypeId(std::string_view type_name) const noexcept = 0;

    /**
     * @brief Get a type name from it's id
     *
     * @param type_id Id of a type
     *
     * @return The name of the type
     */
    virtual std::string_view TypeName(DataType type_id) const noexcept = 0;

    /**
     * @brief Get the list of all types defined by the capability
     *
     * @return A span of the a struct containing the id & name of the types
     * If the span is empty, it means the capability has not yet registered it's
     * types.
     */
    virtual std::span<const TypeDefinition> Types() const noexcept = 0;
};

/**
 * @brief Reusable type list capability.
 *
 * Modules register their types before the core assigns IDs to them.
 */
class TypeListCapability final : public ITypeListCapability {
   public:
    TypeListCapability() = default;
    ~TypeListCapability() override = default;

    /**
     * @brief Register a new type for the module.
     *
     * @param name The name of the type.
     */
    void RegisterType(std::string name);

    const std::string_view *RegisterType(
        DataType type_id) const noexcept override;
    DataType TypeId(std::string_view type_name) const noexcept override;
    std::string_view TypeName(DataType type_id) const noexcept override;
    std::span<const TypeDefinition> Types() const noexcept override;

   private:
    std::list<std::string> owned_names_;
    std::vector<std::string_view> pending_types_;
    mutable std::size_t next_type_index_ = 0;
    mutable std::vector<TypeDefinition> registered_types_;
    mutable std::unordered_map<std::string_view, DataType> name_to_id_;
    mutable std::unordered_map<DataType, std::string_view> id_to_name_;
};

}  // namespace capa
}  // namespace core
