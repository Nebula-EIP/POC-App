#pragma once

#include <filesystem>
#include <functional>
#include <optional>

namespace editor::project {

enum class SaveChoice { kYes, kNo, kCancel };

/// Called repeatedly while a dialog is open, so the window keeps being drawn.
using DialogPump = std::function<void()>;

/// @return The chosen path, or nothing when cancelled.
[[nodiscard]] std::optional<std::filesystem::path> PickSaveFile(
    const std::filesystem::path &start_folder, const DialogPump &pump);

[[nodiscard]] std::optional<std::filesystem::path> PickOpenFile(
    const std::filesystem::path &start_folder, const DialogPump &pump);

[[nodiscard]] SaveChoice AskSaveChanges(const DialogPump &pump);

}  // namespace editor::project
