/**
 * @file shared_library.hpp
 * @brief Thin RAII wrapper around the platform shared library API
 * (dlopen/dlsym/dlclose on POSIX, LoadLibrary/GetProcAddress/FreeLibrary on
 * Windows).
 *
 * This helper is intentionally free of any Nebula concept: it never throws and
 * never logs. Callers translate its boolean results and LastError() message
 * into the module exceptions documented in
 * exception/module_exception/module_exception.md.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 13-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 13-09-2026
 */

#pragma once

#include <filesystem>
#include <string>

namespace core {
namespace detail {

/**
 * @brief Owns an operating system shared library handle.
 *
 * The handle is closed by the destructor. Every object created by the library
 * (in practice the IModule instance returned by its CreateModule factory) must
 * be destroyed *before* the SharedLibrary that owns its code, otherwise the
 * destructor call would jump into unmapped memory.
 */
class SharedLibrary {
   public:
    SharedLibrary() noexcept = default;

    /**
     * @brief Closes the handle if one is still open.
     */
    ~SharedLibrary();

    SharedLibrary(const SharedLibrary &) = delete;
    SharedLibrary &operator=(const SharedLibrary &) = delete;

    SharedLibrary(SharedLibrary &&other) noexcept;
    SharedLibrary &operator=(SharedLibrary &&other) noexcept;

    /**
     * @brief Opens a shared library.
     *
     * A previously opened handle is closed first.
     *
     * @param path Path to the shared library. Required.
     *
     * @return true when the library was opened, false otherwise. On failure,
     * LastError() describes the operating system error.
     */
    bool Open(const std::filesystem::path &path) noexcept;

    /**
     * @brief Closes the handle. Does nothing when no handle is open.
     */
    void Close() noexcept;

    /**
     * @brief Tells whether a handle is currently open.
     *
     * @return true when a handle is open, false otherwise.
     */
    bool IsOpen() const noexcept;

    /**
     * @brief Resolves a symbol exported by the library.
     *
     * @param name Symbol name, as exported with C linkage. Required.
     *
     * @return The address of the symbol, or nullptr when it is not exported or
     * when no library is open. The returned address stays valid until this
     * object is closed, moved from or destroyed.
     */
    void *Symbol(const char *name) const noexcept;

    /**
     * @brief Last error reported by the platform loader.
     *
     * Only meaningful right after Open() or Symbol() returned a failure.
     *
     * @return A human readable description of the error, or an empty string
     * when the platform reported none.
     */
    static std::string LastError();

   private:
    /// Opaque platform handle (void * on POSIX, HMODULE on Windows).
    void *handle_ = nullptr;
};

}  // namespace detail
}  // namespace core
