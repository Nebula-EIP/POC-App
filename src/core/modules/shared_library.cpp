/**
 * @file shared_library.cpp
 * @brief Platform implementation of the SharedLibrary RAII wrapper.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 13-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 13-09-2026
 */

#include "shared_library.hpp"

#include <utility>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace core::detail {

SharedLibrary::~SharedLibrary() { Close(); }

SharedLibrary::SharedLibrary(SharedLibrary &&other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)) {}

SharedLibrary &SharedLibrary::operator=(SharedLibrary &&other) noexcept {
    if (this != &other) {
        Close();
        handle_ = std::exchange(other.handle_, nullptr);
    }
    return *this;
}

bool SharedLibrary::IsOpen() const noexcept { return handle_ != nullptr; }

#if defined(_WIN32)

bool SharedLibrary::Open(const std::filesystem::path &path) noexcept {
    Close();

    std::error_code error;
    std::filesystem::path absolute = std::filesystem::absolute(path, error);
    const std::filesystem::path &target = error ? path : absolute;

    // LOAD_WITH_ALTERED_SEARCH_PATH makes Windows look for the dependencies of
    // the module next to the module itself instead of next to the executable.
    handle_ = reinterpret_cast<void *>(
        LoadLibraryExW(target.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH));
    return handle_ != nullptr;
}

void SharedLibrary::Close() noexcept {
    if (handle_ != nullptr) {
        FreeLibrary(reinterpret_cast<HMODULE>(handle_));
        handle_ = nullptr;
    }
}

void *SharedLibrary::Symbol(const char *name) const noexcept {
    if (handle_ == nullptr || name == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<void *>(
        GetProcAddress(reinterpret_cast<HMODULE>(handle_), name));
}

std::string SharedLibrary::LastError() {
    const DWORD kCode = GetLastError();
    if (kCode == 0) {
        return {};
    }

    LPSTR buffer = nullptr;
    const DWORD kSize = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, kCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&buffer), 0, nullptr);

    std::string message;
    if (kSize != 0 && buffer != nullptr) {
        message.assign(buffer, kSize);
    } else {
        message = "Windows error " + std::to_string(kCode);
    }
    if (buffer != nullptr) {
        LocalFree(buffer);
    }
    while (!message.empty() &&
           (message.back() == '\n' || message.back() == '\r')) {
        message.pop_back();
    }
    return message;
}

#else

bool SharedLibrary::Open(const std::filesystem::path &path) noexcept {
    Close();

    // Clear any stale error so that LastError() only reports ours.
    dlerror();
    // RTLD_LOCAL keeps the module symbols private, which is what lets several
    // modules define the same symbols without colliding.
    handle_ = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    return handle_ != nullptr;
}

void SharedLibrary::Close() noexcept {
    if (handle_ != nullptr) {
        dlclose(handle_);
        handle_ = nullptr;
    }
}

void *SharedLibrary::Symbol(const char *name) const noexcept {
    if (handle_ == nullptr || name == nullptr) {
        return nullptr;
    }
    dlerror();
    void *address = dlsym(handle_, name);
    if (address == nullptr) {
        return nullptr;
    }
    // A symbol may legitimately resolve to a null address, so dlerror() is the
    // authoritative failure signal.
    if (dlerror() != nullptr) {
        return nullptr;
    }
    return address;
}

std::string SharedLibrary::LastError() {
    const char *error = dlerror();
    return error == nullptr ? std::string{} : std::string{error};
}

#endif

}  // namespace core::detail
