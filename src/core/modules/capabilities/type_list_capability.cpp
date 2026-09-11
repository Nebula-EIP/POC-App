/**
 * @file type_list_capability.cpp
 * @brief Implementation of the type list capability for modules.
 *
 * @author Created by mathys-f
 * @date Created on 07-09-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 07-09-2026
 */

#include "type_list_capability.hpp"

namespace core::capa {

void TypeListCapability::RegisterType(std::string name) {
    for (std::string_view t : pending_types_) {
        if (t == name) return;
    }
    for (const auto &t : registered_types_) {
        if (t.name_ == name) return;
    }
    owned_names_.push_back(std::move(name));
    pending_types_.push_back(owned_names_.back());
}

const std::string_view *TypeListCapability::RegisterType(
    DataType type_id) const noexcept {
    if (next_type_index_ >= pending_types_.size()) {
        return nullptr;
    }
    std::string_view name = pending_types_[next_type_index_];
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
