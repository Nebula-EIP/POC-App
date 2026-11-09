// Keep Windows headers lean and without the min/max macros. Must come before
// portable-file-dialogs.h, which includes <windows.h>.
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#include "project/file_dialogs.hpp"

#include <portable-file-dialogs.h>

#include <string>
#include <vector>

namespace editor::project {

namespace {
const std::vector<std::string> kNebulaFilter = {"Nebula project (*.nebula)",
                                                "*.nebula"};

template <typename Dialog>
auto WaitFor(Dialog &dialog, const DialogPump &pump) {
    while (!dialog.ready(16)) {
        if (pump) {
            pump();
        }
    }
    return dialog.result();
}
}  // namespace

std::optional<std::filesystem::path> PickSaveFile(
    const std::filesystem::path &start_folder, const DialogPump &pump) {
    pfd::save_file dialog("Save project", start_folder.string(), kNebulaFilter);
    const std::string kChosen = WaitFor(dialog, pump);
    if (kChosen.empty()) {
        return std::nullopt;
    }
    return std::filesystem::path(kChosen);
}

std::optional<std::filesystem::path> PickOpenFile(
    const std::filesystem::path &start_folder, const DialogPump &pump) {
    pfd::open_file dialog("Open project", start_folder.string(), kNebulaFilter);
    const std::vector<std::string> kSelected = WaitFor(dialog, pump);
    if (kSelected.empty()) {
        return std::nullopt;
    }
    return std::filesystem::path(kSelected.front());
}

SaveChoice AskSaveChanges(const DialogPump &pump) {
    pfd::message box("Nebula",
                     "The project has unsaved changes. Save them first?",
                     pfd::choice::yes_no_cancel, pfd::icon::question);
    switch (WaitFor(box, pump)) {
        case pfd::button::yes:
            return SaveChoice::kYes;
        case pfd::button::no:
            return SaveChoice::kNo;
        default:
            return SaveChoice::kCancel;
    }
}

}  // namespace editor::project