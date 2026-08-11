#pragma once

#include <string>
#include <span>
#include <unordered_map>

#include "datatypes.hpp"

namespace core {

namespace capa {

class ITypeListCapability {
    struct TypeDefinition {
        DataType id;
        std::string_view name;
    };
   public:
    virtual ~ITypeListCapability() = 0;

    virtual bool registerType(DataType type_id) const noexcept = 0;

    virtual DataType typeId(std::string_view type_name) const noexcept = 0;
    virtual std::string_view typeName(DataType type_id) const noexcept = 0;

    virtual std::span<const TypeDefinition> types() const noexcept = 0;

   private:
    std::unordered_map<DataType, std::string_view> types;
};

}  // namespace capa

}  // namespace core
