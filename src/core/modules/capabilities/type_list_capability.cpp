/**
 * @file type_list_capability.cpp
 * @brief Implementation of the type list capability for modules.
 *
 * @author Created by mathys-f
 * @date Created on 07-09-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 17-09-2026
 */

#include "type_list_capability.hpp"

#include <algorithm>

#include "exception/capabilities_exception/type_list_capability_exception.hpp"

namespace core::capa {

TypeListCapability::TypeListCapability(
    const std::vector<std::string_view> &types_list) {
    auto pos = types_list.begin();
    unregistered_types_.reserve(types_list.size());
    registered_types_.reserve(types_list.size());

    /// Initial value, avoid the automatic failure of the duplicate names test
    unregistered_types_.push_back(*pos);
    pos++;

    while (pos != types_list.end()) {
        /// Check for duplicates names
        if (std::find_if(unregistered_types_.begin(), unregistered_types_.end(),
                         [pos](const std::string_view &name) {
                             return (*pos) == name;
                         }) != unregistered_types_.end()) {
            throw DuplicateTypeNameException(std::string("Duplicate name: ") +
                                             std::string(*pos));
        }

        unregistered_types_.push_back(*pos);
        pos++;
    }
}

std::string_view *TypeListCapability::RegisterType(DataType type_id) noexcept {
    if (next_type_index_ >= unregistered_types_.size()) {
        return nullptr;
    }
    std::string_view name = unregistered_types_[next_type_index_];
    registered_types_.push_back({type_id, name});
    name_to_id_[name] = type_id;
    id_to_name_[type_id] = name;
    next_type_index_++;

    return &registered_types_.back().name_;
}

DataType TypeListCapability::TypeId(std::string_view type_name) const noexcept {
    auto it = name_to_id_.find(type_name);
    if (it != name_to_id_.end()) {
        return it->second;
    }
    return 0;  // Returning 0 (invalid/not found)
}

std::string_view TypeListCapability::TypeName(DataType type_id) const noexcept {
    auto it = id_to_name_.find(type_id);
    if (it != id_to_name_.end()) {
        return it->second;
    }
    return "";
}

std::span<const ITypeListCapability::TypeDefinition> TypeListCapability::Types()
    const noexcept {
    return registered_types_;
}

}  // namespace core::capa
