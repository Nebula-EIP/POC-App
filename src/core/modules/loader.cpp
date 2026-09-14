/**
 * @file loader.cpp
 * @brief ModuleLoader implementation.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 13-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 13-09-2026
 */

#include "loader.hpp"

#include <string>
#include <system_error>
#include <utility>

#include "exception/module_exception/module_load_exception.hpp"
#include "exception/module_exception/module_validation_exception.hpp"

namespace core {

namespace {

/**
 * @brief Build the trailing " (path)" part of every error message.
 *
 * @param path Path of the module being loaded.
 *
 * @return The formatted suffix.
 */
std::string Where(const std::filesystem::path &path) {
    return " (" + path.string() + ")";
}

/**
 * @brief Append the operating system error to a message, when there is one.
 *
 * @param message Base message.
 *
 * @return The message, followed by the platform error if any.
 */
std::string WithSystemError(std::string message) {
    const std::string kError = detail::SharedLibrary::LastError();
    if (!kError.empty()) {
        message += ": " + kError;
    }
    return message;
}

}  // namespace

ModuleLoader::Entry::~Entry() {
    // Destruction order is the whole point of this type: shut the module down,
    // destroy it, and only then close the library that owns its code.
    if (instance && initialized) {
        instance->shutdown();
    }
    instance.reset();
    library.Close();
}

ModuleLoader::~ModuleLoader() { unloadAll(); }

std::size_t ModuleLoader::size() const noexcept { return _entries.size(); }

const ModuleLoader::Entry *ModuleLoader::find(ModuleId id) const noexcept {
    for (const std::unique_ptr<Entry> &entry : _entries) {
        if (entry->instance && entry->instance->id() == id) {
            return entry.get();
        }
    }
    return nullptr;
}

const ModuleLoader::Entry *ModuleLoader::find(
    std::string_view name) const noexcept {
    for (const std::unique_ptr<Entry> &entry : _entries) {
        if (entry->instance && entry->instance->name() == name) {
            return entry.get();
        }
    }
    return nullptr;
}

IModule *ModuleLoader::module(ModuleId id) noexcept {
    const Entry *entry = find(id);
    return entry == nullptr ? nullptr : entry->instance.get();
}

IModule *ModuleLoader::module(std::string_view name) noexcept {
    const Entry *entry = find(name);
    return entry == nullptr ? nullptr : entry->instance.get();
}

const IModule *ModuleLoader::module(ModuleId id) const noexcept {
    const Entry *entry = find(id);
    return entry == nullptr ? nullptr : entry->instance.get();
}

const IModule *ModuleLoader::module(std::string_view name) const noexcept {
    const Entry *entry = find(name);
    return entry == nullptr ? nullptr : entry->instance.get();
}

std::span<const IModule *const> ModuleLoader::modules() noexcept {
    return _module_view;
}

void ModuleLoader::refreshCaches() {
    // Every previously returned capability span points into a vector that may
    // now describe unloaded modules.
    _capability_cache.clear();

    _module_view.clear();
    _module_view.reserve(_entries.size());
    for (const std::unique_ptr<Entry> &entry : _entries) {
        _module_view.push_back(entry->instance.get());
    }
}

ModuleId ModuleLoader::load(std::filesystem::path path) {
    if (_next_id == 0) {
        throw ModuleException("The module id space is exhausted");
    }

    std::error_code error;
    if (path.empty() || !std::filesystem::exists(path, error) || error) {
        throw ModuleFileNotFoundException("Module file was not found" +
                                          Where(path));
    }
    if (!std::filesystem::is_regular_file(path, error) || error) {
        throw ModuleFileNotFoundException("Module path is not a regular file" +
                                          Where(path));
    }

    std::filesystem::path canonical =
        std::filesystem::weakly_canonical(path, error);
    if (error) {
        canonical = path;
    }
    for (const std::unique_ptr<Entry> &loaded : _entries) {
        if (loaded->path == canonical) {
            throw ModuleAlreadyLoadedException(
                "The module library is already loaded" + Where(path));
        }
    }

    // From here on the entry owns everything the attempt creates, so any throw
    // releases the module and the library, in that order.
    auto entry = std::make_unique<Entry>();
    entry->path = std::move(canonical);

    if (!entry->library.Open(path)) {
        throw ModuleLoadFailedException(
            WithSystemError("Failed to load the module library" + Where(path)));
    }

    auto factory = reinterpret_cast<CreateModuleFunction>(
        entry->library.Symbol(kCreateModuleSymbol));
    if (factory == nullptr) {
        throw ModuleSymbolNotFoundException(std::string{kCreateModuleSymbol} +
                                            " symbol was not found" +
                                            Where(path));
    }

    entry->instance.reset(factory());
    if (!entry->instance) {
        throw InvalidModuleException(std::string{kCreateModuleSymbol} +
                                     " returned no module" + Where(path));
    }

    const std::string_view kName = entry->instance->name();
    if (kName.empty()) {
        throw InvalidModuleException("The module has no name" + Where(path));
    }
    if (find(kName) != nullptr) {
        throw ModuleAlreadyLoadedException("A module named '" +
                                           std::string{kName} +
                                           "' is already loaded" + Where(path));
    }

    const ModuleId kId = _next_id;
    if (!entry->instance->initialize(kId)) {
        throw ModuleInitializationException("Failed to initialize module '" +
                                            std::string{kName} + "'" +
                                            Where(path));
    }
    entry->initialized = true;

    if (entry->instance->id() != kId) {
        throw InvalidModuleException("Module '" + std::string{kName} +
                                     "' does not report the id it was given" +
                                     Where(path));
    }
    if (entry->instance->types() == nullptr) {
        throw InvalidModuleException("Module '" + std::string{kName} +
                                     "' provides no type list capability" +
                                     Where(path));
    }
    if (entry->instance->nodes() == nullptr) {
        throw InvalidModuleException("Module '" + std::string{kName} +
                                     "' provides no node list capability" +
                                     Where(path));
    }

    // Commit. Reserving first makes the two insertions non throwing, so the
    // module cannot end up initialized but untracked.
    _entries.reserve(_entries.size() + 1);
    _module_view.reserve(_entries.size() + 1);
    _entries.push_back(std::move(entry));
    refreshCaches();
    _next_id = kId + 1;

    return kId;
}

bool ModuleLoader::unload(ModuleId id) {
    for (auto it = _entries.begin(); it != _entries.end(); ++it) {
        if ((*it)->instance && (*it)->instance->id() == id) {
            _entries.erase(it);
            refreshCaches();
            return true;
        }
    }
    return false;
}

bool ModuleLoader::unload(std::string_view name) {
    for (auto it = _entries.begin(); it != _entries.end(); ++it) {
        if ((*it)->instance && (*it)->instance->name() == name) {
            _entries.erase(it);
            refreshCaches();
            return true;
        }
    }
    return false;
}

void ModuleLoader::unloadAll() {
    _capability_cache.clear();
    _module_view.clear();
    // Reverse load order, so that a module loaded later can still rely on an
    // earlier one while it shuts down.
    while (!_entries.empty()) {
        _entries.pop_back();
    }
}

}  // namespace core
