#pragma once

#include <memory>
#include <string_view>
#include <typeindex>
#include <cstdint>

namespace core {

class ICapability;

using ModuleId = uint32_t;

/**
 * @brief Modules will be loaded by the module loader class.
 * They contains a set of capabilities.
 */
class IModule {
   public:
    virtual ~IModule() = default;

    virtual std::string_view name() const noexcept = 0;
    virtual std::string_view author() const noexcept;
    virtual std::string_view description() const noexcept;

    struct Version {
        uint16_t major;
        uint16_t minor;
        uint16_t patch;
    };
    virtual Version version() const noexcept = 0;

    /**
     * @brief The id in the param here is just to inform the module of the id the loader has given him.
     */
    virtual bool initialize(ModuleId id) = 0;
    virtual void shutdown() noexcept = 0;

    virtual ModuleId id() const noexcept = 0;

    // virtual TypeListCapability& types() = 0;
    // virtual NodeListCapability& nodes() = 0;

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
