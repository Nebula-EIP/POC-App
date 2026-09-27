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
    if (instance_ && initialized_) {
        instance_->Shutdown();
    }
    instance_.reset();
    library_.Close();
}

ModuleLoader::~ModuleLoader() { UnloadAll(); }

std::size_t ModuleLoader::Size() const noexcept { return entries_.size(); }

const ModuleLoader::Entry *ModuleLoader::Find(ModuleId id) const noexcept {
    for (const std::unique_ptr<Entry> &entry : entries_) {
        if (entry->instance_ && entry->instance_->Id() == id) {
            return entry.get();
        }
    }
    return nullptr;
}

const ModuleLoader::Entry *ModuleLoader::Find(
    std::string_view name) const noexcept {
    for (const std::unique_ptr<Entry> &entry : entries_) {
        if (entry->instance_ && entry->instance_->Name() == name) {
            return entry.get();
        }
    }
    return nullptr;
}

IModule *ModuleLoader::Module(ModuleId id) noexcept {
    const Entry *entry = Find(id);
    return entry == nullptr ? nullptr : entry->instance_.get();
}

IModule *ModuleLoader::Module(std::string_view name) noexcept {
    const Entry *entry = Find(name);
    return entry == nullptr ? nullptr : entry->instance_.get();
}

const IModule *ModuleLoader::Module(ModuleId id) const noexcept {
    const Entry *entry = Find(id);
    return entry == nullptr ? nullptr : entry->instance_.get();
}

const IModule *ModuleLoader::Module(std::string_view name) const noexcept {
    const Entry *entry = Find(name);
    return entry == nullptr ? nullptr : entry->instance_.get();
}

std::span<const IModule *const> ModuleLoader::Modules() noexcept {
    return module_view_;
}

void ModuleLoader::RefreshCaches() {
    // Every previously returned capability span points into a vector that may
    // now describe unloaded modules.
    capability_cache_.clear();

    module_view_.clear();
    module_view_.reserve(entries_.size());
    for (const std::unique_ptr<Entry> &entry : entries_) {
        module_view_.push_back(entry->instance_.get());
    }
}

ModuleId ModuleLoader::Load(std::filesystem::path path) {
    if (next_id_ == 0) {
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
    for (const std::unique_ptr<Entry> &loaded : entries_) {
        if (loaded->path_ == canonical) {
            throw ModuleAlreadyLoadedException(
                "The module library is already loaded" + Where(path));
        }
    }

    // From here on the entry owns everything the attempt creates, so any throw
    // releases the module and the library, in that order.
    auto entry = std::make_unique<Entry>();
    entry->path_ = std::move(canonical);

    if (!entry->library_.Open(path)) {
        throw ModuleLoadFailedException(
            WithSystemError("Failed to load the module library" + Where(path)));
    }

    auto factory = reinterpret_cast<CreateModuleFunction>(
        entry->library_.Symbol(kCreateModuleSymbol));
    if (factory == nullptr) {
        throw ModuleSymbolNotFoundException(std::string{kCreateModuleSymbol} +
                                            " symbol was not found" +
                                            Where(path));
    }

    entry->instance_.reset(factory());
    if (!entry->instance_) {
        throw InvalidModuleException(std::string{kCreateModuleSymbol} +
                                     " returned no module" + Where(path));
    }

    const std::string_view kName = entry->instance_->Name();
    if (kName.empty()) {
        throw InvalidModuleException("The module has no name" + Where(path));
    }
    if (Find(kName) != nullptr) {
        throw ModuleAlreadyLoadedException("A module named '" +
                                           std::string{kName} +
                                           "' is already loaded" + Where(path));
    }

    const ModuleId kId = next_id_;
    if (!entry->instance_->Initialize(kId)) {
        throw ModuleInitializationException("Failed to initialize module '" +
                                            std::string{kName} + "'" +
                                            Where(path));
    }
    entry->initialized_ = true;

    if (entry->instance_->Types() == nullptr) {
        throw InvalidModuleException("Module '" + std::string{kName} +
                                     "' provides no type list capability" +
                                     Where(path));
    }
    if (entry->instance_->Nodes() == nullptr) {
        throw InvalidModuleException("Module '" + std::string{kName} +
                                     "' provides no node list capability" +
                                     Where(path));
    }

    if (entry->instance_->Id() != kId) {
        throw InvalidModuleException("Module '" + std::string{kName} +
                                     "' does not report the id it was given" +
                                     Where(path));
    }

    // Commit. Reserving first makes the two insertions non throwing, so the
    // module cannot end up initialized but untracked.
    entries_.reserve(entries_.size() + 1);
    module_view_.reserve(entries_.size() + 1);
    entries_.push_back(std::move(entry));
    RefreshCaches();
    next_id_ = kId + 1;

    return kId;
}

bool ModuleLoader::Unload(ModuleId id) {
    for (auto it = entries_.begin(); it != entries_.end(); ++it) {
        if ((*it)->instance_ && (*it)->instance_->Id() == id) {
            entries_.erase(it);
            RefreshCaches();
            return true;
        }
    }
    return false;
}

bool ModuleLoader::Unload(std::string_view name) {
    for (auto it = entries_.begin(); it != entries_.end(); ++it) {
        if ((*it)->instance_ && (*it)->instance_->Name() == name) {
            entries_.erase(it);
            RefreshCaches();
            return true;
        }
    }
    return false;
}

void ModuleLoader::UnloadAll() {
    capability_cache_.clear();
    module_view_.clear();
    // Reverse load order, so that a module loaded later can still rely on an
    // earlier one while it shuts down.
    while (!entries_.empty()) {
        entries_.pop_back();
    }
}

}  // namespace core
