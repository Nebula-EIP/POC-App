/**
 * @file fixture_module.cpp
 * @brief Single source used to build every test module fixture.
 *
 * Each CMake target compiles this file with a different set of
 * FIXTURE_* definitions, which is enough to cover every load path the loader
 * has to handle without duplicating the IModule boilerplate.
 *
 * This fixture is deliberately independent from raylib and from the editor: it
 * opens no window and touches no global state.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 13-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 13-09-2026
 */

#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>

#include "modules/capabilities/node_list_capability.hpp"
#include "modules/capabilities/renderer_capability.hpp"
#include "modules/capabilities/type_list_capability.hpp"
#include "modules/create_module.hpp"
#include "modules/module.hpp"

/// Name reported by IModule::Name().
#ifndef FIXTURE_MODULE_NAME
#define FIXTURE_MODULE_NAME "FixtureModule"
#endif

/// Value returned by IModule::Initialize().
#ifndef FIXTURE_INITIALIZE_RESULT
#define FIXTURE_INITIALIZE_RESULT true
#endif

/// 1 when the mandatory type list capability is provided.
#ifndef FIXTURE_PROVIDE_TYPES
#define FIXTURE_PROVIDE_TYPES 1
#endif

/// 1 when the mandatory node list capability is provided.
#ifndef FIXTURE_PROVIDE_NODES
#define FIXTURE_PROVIDE_NODES 1
#endif

/// 1 when the optional renderer capability is provided.
#ifndef FIXTURE_PROVIDE_RENDERER
#define FIXTURE_PROVIDE_RENDERER 0
#endif

/// 1 when CreateModule must return nullptr.
#ifndef FIXTURE_NULL_FACTORY
#define FIXTURE_NULL_FACTORY 0
#endif

/// 1 when the module reports an id different from the one it was given.
#ifndef FIXTURE_WRONG_ID
#define FIXTURE_WRONG_ID 0
#endif

/// 1 when the library must not export the CreateModule factory at all.
#ifndef FIXTURE_NO_FACTORY
#define FIXTURE_NO_FACTORY 0
#endif

namespace {

/// Lifecycle trace, read back by the tests through FixtureEvents(). 'C' is a
/// construction, 'S' a Shutdown() call and 'D' a destruction.
std::string g_events;  // NOLINT(runtime/string)

/**
 * @brief Append one lifecycle event to the trace.
 *
 * @param event Event character.
 */
void trace(char event) { g_events.push_back(event); }

/**
 * @brief Minimal but complete IModule implementation.
 */
class FixtureModule final : public core::IModule {
   public:
    FixtureModule()
        : types_(std::vector<std::string_view>({"FixtureType"})) {
        trace('C');
        nodes_.RegisterNode("FixtureNode", "A node provided by a test fixture",
                            {});
#if FIXTURE_PROVIDE_RENDERER
        renderer_.RegisterNodeRenderer(
            0,
            [](core::NodeId, core::NodeType, const core::PropertyMap &)
                -> core::capa::ComponentList { return {}; });
#endif
    }

    ~FixtureModule() override { trace('D'); }

    std::string_view Name() const noexcept override { return name_; }

    std::string_view Author() const noexcept override { return "tests"; }

    std::string_view Description() const noexcept override {
        return "Test module fixture";
    }

    Version GetVersion() const noexcept override { return {1, 0, 0}; }

    bool Initialize(core::ModuleId id) override {
        if (!static_cast<bool>(FIXTURE_INITIALIZE_RESULT)) {
            return false;
        }
#if FIXTURE_WRONG_ID
        id_ = static_cast<core::ModuleId>(id + 1);
#else
        id_ = id;
#endif
        return true;
    }

    void Shutdown() noexcept override {
        trace('S');
        id_ = 0;
    }

    core::ModuleId Id() const noexcept override { return id_; }

    core::capa::ITypeListCapability *Types() override {
#if FIXTURE_PROVIDE_TYPES
        return &types_;
#else
        return nullptr;
#endif
    }

    core::capa::INodeListCapability *Nodes() override {
#if FIXTURE_PROVIDE_NODES
        return &nodes_;
#else
        return nullptr;
#endif
    }

   private:
    core::ICapability *Capability(std::type_index type) noexcept override {
        return const_cast<core::ICapability *>(
            static_cast<const FixtureModule *>(this)->Capability(type));
    }

    const core::ICapability *Capability(
        std::type_index type) const noexcept override {
#if FIXTURE_PROVIDE_TYPES
        if (type == std::type_index(typeid(core::capa::ITypeListCapability))) {
            return &types_;
        }
#endif
#if FIXTURE_PROVIDE_NODES
        if (type == std::type_index(typeid(core::capa::INodeListCapability))) {
            return &nodes_;
        }
#endif
#if FIXTURE_PROVIDE_RENDERER
        if (type == std::type_index(typeid(core::capa::IRendererCapability))) {
            return &renderer_;
        }
#endif
        (void)type;
        return nullptr;
    }

    std::string name_ = FIXTURE_MODULE_NAME;
    core::ModuleId id_ = 0;
    core::capa::TypeListCapability types_;
    core::capa::NodeListCapability nodes_;
    core::capa::RendererCapability renderer_;
};

}  // namespace

extern "C" {

/**
 * @brief Lifecycle trace of every FixtureModule created by this library.
 *
 * @return A NUL terminated string, valid until the next call to
 * FixtureResetEvents() or until the library is closed.
 */
NEBULA_MODULE_EXPORT const char *FixtureEvents() { return g_events.c_str(); }

/**
 * @brief Clear the lifecycle trace.
 */
NEBULA_MODULE_EXPORT void FixtureResetEvents() { g_events.clear(); }
}

#if FIXTURE_NO_FACTORY

// This fixture exports something, just not the factory the loader looks for.
extern "C" NEBULA_MODULE_EXPORT int NotTheFactory() { return 0; }

#else

extern "C" NEBULA_MODULE_EXPORT core::IModule *CreateModule() {
#if FIXTURE_NULL_FACTORY
    return nullptr;
#else
    return new FixtureModule();
#endif
}

#endif
