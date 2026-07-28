#pragma once

#include <memory>
#include <string_view>
#include <typeindex>

namespace core {

class ICapability;

/**
 * @brief Modules will be loaded by the module loader class.
 * They contains a set of capabilities.
 */
class IModule {
   public:
    virtual ~IModule() = default;

    virtual std::string_view name() const noexcept = 0;
    virtual std::string_view version() const noexcept = 0;

    virtual bool initialize() = 0;
    virtual void shutdown() noexcept = 0;

    virtual ICapability* capability(std::type_index type) noexcept = 0;
    virtual const ICapability* capability(std::type_index type) const noexcept = 0;

    template<typename T>
    T* capability() noexcept
    {
        static_assert(std::is_base_of_v<ICapability, T>);
        return static_cast<T*>(capability(typeid(T)));
    }

    template<typename T>
    const T* capability() const noexcept
    {
        static_assert(std::is_base_of_v<ICapability, T>);
        return static_cast<const T*>(capability(typeid(T)));
    }

   private:
};

} // namespace core
