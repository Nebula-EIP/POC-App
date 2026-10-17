/**
 * @file node_picker.cpp
 * @brief Implementation of the NodePicker class.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 11-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 11-10-2026
 */

#include "node_picker.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string_view>
#include <utility>

namespace editor::ui {

namespace {
constexpr std::size_t kMaxVisibleRows = 12;
constexpr float kPadding = 4.0F;
constexpr float kSearchMargin = 6.0F;
constexpr float kIndent = 14.0F;
constexpr float kArrowWidth = 14.0F;
constexpr float kScrollbarWidth = 4.0F;
constexpr float kShadowOffset = 3.0F;
constexpr utils::WrappedColor kShadow = {0, 0, 0, 70};
constexpr utils::WrappedColor kSearchBackground = {24, 24, 28, 255};
constexpr utils::WrappedColor kFolderText = {140, 160, 200, 255};
constexpr const char *kPlaceholder = "Rechercher...";
constexpr const char *kNoResult = "Aucun node";
constexpr char kCategorySeparator = '/';
constexpr int kFirstPrintable = 32;
constexpr int kLastPrintable = 126;

std::string Lower(std::string_view text) {
    std::string lower(text);
    std::ranges::transform(lower, lower.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return lower;
}

bool Contains(utils::WrappedRectangle rectangle, utils::WrappedVector2 point) {
    return point.x_ >= rectangle.x_ &&
           point.x_ < rectangle.x_ + rectangle.width_ &&
           point.y_ >= rectangle.y_ &&
           point.y_ < rectangle.y_ + rectangle.height_;
}

/// Splits "Flow/Loops" into {"Flow", "Loops"}, dropping empty parts.
std::vector<std::string> SplitCategory(std::string_view category) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (start <= category.size()) {
        const std::size_t kEnd = category.find(kCategorySeparator, start);
        const std::size_t kStop =
            kEnd == std::string_view::npos ? category.size() : kEnd;
        if (kStop > start) {
            parts.emplace_back(category.substr(start, kStop - start));
        }
        start = kStop + 1;
    }
    return parts;
}
}  // namespace

std::vector<NodePickerEntry> EntriesFromCatalog(
    const core::NodeCatalog &catalog,
    const std::function<std::string(core::ModuleId)> &module_name) {
    std::vector<NodePickerEntry> entries;
    for (const core::NodeDescriptor &node : catalog.GetNodes()) {
        entries.push_back({node.id_, node.name_, module_name(node.module_id_),
                           node.category_});
    }
    return entries;
}

// ---------------------------------------------------------------------------
// Content

NodePicker::NodePicker(TopBarStyle style, TextMeasurer measurer)
    : style_(style), measurer_(std::move(measurer)) {
    if (!measurer_) {
        measurer_ = [](const std::string &text, int font_size) {
            return utils::MeasureTextWrapped(text.c_str(), font_size);
        };
    }
    width_ = ComputeWidth();
}

void NodePicker::SetEntries(std::vector<NodePickerEntry> entries) {
    entries_ = std::move(entries);
    BuildTree();
    width_ = ComputeWidth();
    expanded_.clear();
    selected_.reset();
    scroll_ = 0;
    RebuildRows();
}

std::size_t NodePicker::FolderChild(std::optional<std::size_t> parent,
                                    const std::string &name) {
    const std::vector<std::size_t> &siblings =
        parent ? folders_[*parent].folders_ : roots_;
    for (const std::size_t kFolder : siblings) {
        if (folders_[kFolder].name_ == name) {
            return kFolder;
        }
    }

    std::string path =
        parent ? folders_[*parent].path_ + kCategorySeparator + name : name;
    const std::size_t kIndex = folders_.size();
    folders_.push_back({name, std::move(path), {}, {}});
    // Indexed again: push_back may have moved the parent.
    (parent ? folders_[*parent].folders_ : roots_).push_back(kIndex);
    return kIndex;
}

void NodePicker::BuildTree() {
    folders_.clear();
    roots_.clear();
    for (std::size_t index = 0; index < entries_.size(); ++index) {
        std::size_t folder = FolderChild(std::nullopt, entries_[index].module_);
        for (const std::string &part :
             SplitCategory(entries_[index].category_)) {
            folder = FolderChild(folder, part);
        }
        folders_[folder].entries_.push_back(index);
    }
}

void NodePicker::AddFolderRows(std::size_t folder, std::size_t depth) {
    const bool kExpanded = IsExpanded(folders_[folder].path_);
    rows_.push_back({RowKind::kFolder, folders_[folder].name_, depth, 0,
                     folders_[folder].path_, kExpanded});
    if (!kExpanded) {
        return;
    }
    // Sub-folders first, then the nodes, like a file browser.
    for (const std::size_t kChild : folders_[folder].folders_) {
        AddFolderRows(kChild, depth + 1);
    }
    for (const std::size_t kEntry : folders_[folder].entries_) {
        rows_.push_back(
            {RowKind::kEntry, entries_[kEntry].name_, depth + 1, kEntry, {}});
    }
}

void NodePicker::AddSearchRows() {
    const std::string kNeedle = Lower(query_);
    // Names starting with the search first, then names containing it.
    for (const bool kPrefixPass : {true, false}) {
        for (std::size_t index = 0; index < entries_.size(); ++index) {
            const std::size_t kFound =
                Lower(entries_[index].name_).find(kNeedle);
            if (kFound == std::string::npos || (kFound == 0) != kPrefixPass) {
                continue;
            }
            rows_.push_back(
                {RowKind::kEntry, entries_[index].name_, 0, index, {}});
        }
    }
}

std::string NodePicker::RowKey(std::size_t row) const {
    if (row >= rows_.size()) {
        return {};
    }
    switch (rows_[row].kind_) {
        case RowKind::kFolder:
            return "folder:" + rows_[row].path_;
        case RowKind::kEntry:
            return "entry:" + std::to_string(rows_[row].entry_);
        case RowKind::kEmpty:
            break;
    }
    return {};
}

void NodePicker::RebuildRows() {
    const std::string kSelectedKey = selected_ ? RowKey(*selected_) : "";

    rows_.clear();
    if (IsSearching()) {
        AddSearchRows();
    } else {
        for (const std::size_t kRoot : roots_) {
            AddFolderRows(kRoot, 0);
        }
    }
    if (rows_.empty()) {
        rows_.push_back({RowKind::kEmpty, kNoResult, 0, 0, {}});
    }

    selected_.reset();
    if (!kSelectedKey.empty()) {
        for (std::size_t row = 0; row < rows_.size(); ++row) {
            if (RowKey(row) == kSelectedKey) {
                selected_ = row;
                break;
            }
        }
    }
    if (!selected_.has_value() && rows_.front().kind_ != RowKind::kEmpty) {
        selected_ = 0;
    }
    scroll_ = std::min(scroll_, MaxScroll());
    EnsureSelectedVisible();
}

// ---------------------------------------------------------------------------
// Opening

void NodePicker::Open(utils::WrappedVector2 position, float screen_width,
                      float screen_height, PickCallback on_pick) {
    query_.clear();
    expanded_.clear();
    // A single module: its folder is opened right away, saving a click.
    if (roots_.size() == 1) {
        expanded_.insert(folders_[roots_.front()].path_);
    }
    selected_.reset();
    scroll_ = 0;
    RebuildRows();
    // Drop characters still queued from the key press that opened the
    // picker (e.g. the "A" of Shift+A), otherwise they land in the search.
    while (utils::GetCharPressedWrapped() > 0) {
    }
    skip_typed = true;
    // Sized for the whole tree, so that the box does not jump while folders
    // are opened or a search is typed.
    visible_rows_ = std::clamp<std::size_t>(folders_.size() + entries_.size(),
                                            1, kMaxVisibleRows);
    on_pick_ = std::move(on_pick);
    last_cursor_.reset();
    is_open_ = true;

    position_ = position;
    const utils::WrappedRectangle kBounds = Bounds();
    if (screen_width > 0.0F && position_.x_ + kBounds.width_ > screen_width) {
        position_.x_ = std::max(0.0F, screen_width - kBounds.width_);
    }
    if (screen_height > 0.0F &&
        position_.y_ + kBounds.height_ > screen_height) {
        position_.y_ = std::max(0.0F, screen_height - kBounds.height_);
    }
}

void NodePicker::Close() noexcept {
    is_open_ = false;
    on_pick_ = nullptr;
}

// ---------------------------------------------------------------------------
// Layout

float NodePicker::TextX(std::size_t depth) const {
    return style_.item_padding_ + static_cast<float>(depth) * kIndent +
           kArrowWidth;
}

float NodePicker::ComputeWidth() const {
    float width = std::max(style_.min_dropdown_width_,
                           measurer_(kPlaceholder, style_.font_size_) +
                               2.0F * (style_.item_padding_ + kSearchMargin));
    // Every folder and node at its deepest position, all folders open.
    const auto kVisit = [&](const auto &self, std::size_t folder,
                            std::size_t depth) -> void {
        width = std::max(
            width, TextX(depth) +
                       measurer_(folders_[folder].name_, style_.font_size_) +
                       style_.item_padding_);
        for (const std::size_t kChild : folders_[folder].folders_) {
            self(self, kChild, depth + 1);
        }
        for (const std::size_t kEntry : folders_[folder].entries_) {
            width = std::max(width, TextX(depth + 1) +
                                        measurer_(entries_[kEntry].name_,
                                                  style_.font_size_) +
                                        style_.item_padding_);
        }
    };
    for (const std::size_t kRoot : roots_) {
        kVisit(kVisit, kRoot, 0);
    }
    return width + kScrollbarWidth;
}

utils::WrappedRectangle NodePicker::SearchRect() const {
    return {position_.x_ + kSearchMargin, position_.y_ + kSearchMargin,
            Width() - 2.0F * kSearchMargin, style_.item_height_};
}

utils::WrappedRectangle NodePicker::Bounds() const {
    const float kHeight =
        2.0F * kSearchMargin + style_.item_height_ +
        static_cast<float>(visible_rows_) * style_.item_height_ + kPadding;
    return {position_.x_, position_.y_, Width(), kHeight};
}

std::optional<utils::WrappedRectangle> NodePicker::RowRect(
    std::size_t row) const {
    if (row < scroll_ || row >= scroll_ + visible_rows_ ||
        row >= rows_.size()) {
        return std::nullopt;
    }
    const utils::WrappedRectangle kSearch = SearchRect();
    const float kTop = kSearch.y_ + kSearch.height_ + kSearchMargin;
    return utils::WrappedRectangle{
        position_.x_,
        kTop + static_cast<float>(row - scroll_) * style_.item_height_,
        Width() - kScrollbarWidth, style_.item_height_};
}

std::optional<std::size_t> NodePicker::HitTestRow(
    utils::WrappedVector2 point) const {
    for (std::size_t row = scroll_;
         row < rows_.size() && row < scroll_ + visible_rows_; ++row) {
        if (const auto kRect = RowRect(row); kRect && Contains(*kRect, point)) {
            return row;
        }
    }
    return std::nullopt;
}

// ---------------------------------------------------------------------------
// Navigation

void NodePicker::SetExpanded(std::size_t row, bool expanded) {
    if (row >= rows_.size() || rows_[row].kind_ != RowKind::kFolder ||
        rows_[row].expanded_ == expanded) {
        return;
    }
    if (expanded) {
        expanded_.insert(rows_[row].path_);
    } else {
        expanded_.erase(rows_[row].path_);
    }
    selected_ = row;  // The toggled folder stays selected.
    RebuildRows();
}

void NodePicker::Activate(std::size_t row) {
    if (row >= rows_.size()) {
        return;
    }
    if (rows_[row].kind_ == RowKind::kFolder) {
        SetExpanded(row, !rows_[row].expanded_);
    } else {
        Pick(row);
    }
}

void NodePicker::MoveSelection(int direction) {
    if (!selected_.has_value()) {
        return;
    }
    if (direction < 0 && *selected_ > 0) {
        --*selected_;
    } else if (direction > 0 && *selected_ + 1 < rows_.size()) {
        ++*selected_;
    }
    EnsureSelectedVisible();
}

void NodePicker::SelectParent() {
    if (!selected_.has_value() || rows_[*selected_].depth_ == 0) {
        return;
    }
    const std::size_t kParentDepth = rows_[*selected_].depth_ - 1;
    for (std::size_t row = *selected_; row-- > 0;) {
        if (rows_[row].kind_ == RowKind::kFolder &&
            rows_[row].depth_ == kParentDepth) {
            selected_ = row;
            EnsureSelectedVisible();
            return;
        }
    }
}

std::size_t NodePicker::MaxScroll() const noexcept {
    return rows_.size() > visible_rows_ ? rows_.size() - visible_rows_ : 0;
}

void NodePicker::ScrollBy(int rows) {
    const std::int64_t kNext = static_cast<std::int64_t>(scroll_) + rows;
    scroll_ = static_cast<std::size_t>(std::clamp<std::int64_t>(
        kNext, 0, static_cast<std::int64_t>(MaxScroll())));
}

void NodePicker::EnsureSelectedVisible() {
    if (!selected_.has_value()) {
        return;
    }
    if (*selected_ < scroll_) {
        scroll_ = *selected_;
    } else if (*selected_ >= scroll_ + visible_rows_) {
        scroll_ = *selected_ + 1 - visible_rows_;
    }
}

void NodePicker::Pick(std::size_t row) {
    if (row >= rows_.size() || rows_[row].kind_ != RowKind::kEntry) {
        return;
    }
    const core::NodeType kType = entries_[rows_[row].entry_].type_;
    PickCallback callback = std::move(on_pick_);
    Close();
    if (callback) {
        callback(kType);
    }
}

// ---------------------------------------------------------------------------
// Input

NodePickerInput NodePicker::ReadInput() {
    NodePickerInput input;
    input.cursor_ = utils::GetCursorPositionWrapped();
    input.left_pressed_ = utils::IsLeftClicked();
    input.right_pressed_ = utils::IsRightClicked();
    input.escape_pressed_ =
        utils::IsKeyPressedWrapped(utils::WrappedKey::kEscape);
    input.enter_pressed_ =
        utils::IsKeyPressedWrapped(utils::WrappedKey::kEnter) ||
        utils::IsKeyPressedWrapped(utils::WrappedKey::kKpEnter);
    input.up_pressed_ = utils::IsKeyPressedWrapped(utils::WrappedKey::kUp);
    input.down_pressed_ = utils::IsKeyPressedWrapped(utils::WrappedKey::kDown);
    input.left_key_pressed_ =
        utils::IsKeyPressedWrapped(utils::WrappedKey::kLeft);
    input.right_key_pressed_ =
        utils::IsKeyPressedWrapped(utils::WrappedKey::kRight);
    input.backspace_pressed_ =
        utils::IsKeyPressedWrapped(utils::WrappedKey::kBackspace);
    for (int character = utils::GetCharPressedWrapped(); character > 0;
         character = utils::GetCharPressedWrapped()) {
        // The default font only draws ASCII. The first read after Open()
        // discards the character of the key that opened the picker.
        if (!skip_typed && character >= kFirstPrintable &&
            character <= kLastPrintable) {
            input.typed_.push_back(static_cast<char>(character));
        }
    }
    skip_typed = false;
    input.wheel_ = utils::GetMouseWheelMoveWrapped();
    input.screen_width_ = utils::GetScreenWidthWrapped();
    return input;
}

bool NodePicker::Update(const NodePickerInput &input) {
    if (!is_open_) {
        return false;
    }
    if (input.escape_pressed_) {
        Close();
        return true;
    }

    // Search field. A new search selects its first result.
    std::string query = query_;
    if (input.backspace_pressed_ && !query.empty()) {
        query.pop_back();
    }
    query += input.typed_;
    if (query != query_) {
        query_ = std::move(query);
        selected_.reset();
        scroll_ = 0;
        RebuildRows();
    }

    // Keyboard navigation.
    if (input.up_pressed_) {
        MoveSelection(-1);
    }
    if (input.down_pressed_) {
        MoveSelection(1);
    }
    if (selected_.has_value() && !IsSearching()) {
        const Row &row = rows_[*selected_];
        if (input.right_key_pressed_ && row.kind_ == RowKind::kFolder) {
            if (row.expanded_) {
                MoveSelection(1);  // Into the open folder.
            } else {
                SetExpanded(*selected_, true);
            }
        } else if (input.left_key_pressed_) {
            if (row.kind_ == RowKind::kFolder && row.expanded_) {
                SetExpanded(*selected_, false);
            } else {
                SelectParent();
            }
        }
    }

    const bool kInside = Contains(Bounds(), input.cursor_);
    if (kInside && input.wheel_ != 0.0F) {
        ScrollBy(input.wheel_ > 0.0F ? -1 : 1);
    }

    // The mouse only takes the selection over when it moves, so that the
    // keyboard selection stays put under a still cursor, including the
    // cursor that opened the picker.
    const bool kCursorMoved =
        last_cursor_.has_value() && (last_cursor_->x_ != input.cursor_.x_ ||
                                     last_cursor_->y_ != input.cursor_.y_);
    last_cursor_ = input.cursor_;
    const std::optional<std::size_t> kHovered = HitTestRow(input.cursor_);
    const bool kHoversRow =
        kHovered.has_value() && rows_[*kHovered].kind_ != RowKind::kEmpty;
    if (kHoversRow && kCursorMoved) {
        selected_ = kHovered;
    }

    if (input.enter_pressed_ && selected_.has_value()) {
        Activate(*selected_);
        return true;
    }

    if (input.left_pressed_ || input.right_pressed_) {
        if (!kInside) {
            // Clicked outside: the click still reaches what is under it.
            Close();
            return false;
        }
        if (input.left_pressed_ && kHoversRow) {
            selected_ = kHovered;
            Activate(*kHovered);
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Drawing

void NodePicker::Draw() const {
    if (!is_open_) {
        return;
    }
    const float kFont = static_cast<float>(style_.font_size_);
    const utils::WrappedRectangle kBounds = Bounds();

    utils::DrawRectangleWrapped(kBounds.x_ + kShadowOffset,
                                kBounds.y_ + kShadowOffset, kBounds.width_,
                                kBounds.height_, kShadow);
    utils::DrawRectangleWrapped(kBounds.x_, kBounds.y_, kBounds.width_,
                                kBounds.height_, style_.dropdown_);
    utils::DrawRectangleLinesWrapped(kBounds.x_, kBounds.y_, kBounds.width_,
                                     kBounds.height_, style_.border_);

    // Search field, with a caret after the text.
    const utils::WrappedRectangle kSearch = SearchRect();
    utils::DrawRectangleWrapped(kSearch.x_, kSearch.y_, kSearch.width_,
                                kSearch.height_, kSearchBackground);
    utils::DrawRectangleLinesWrapped(kSearch.x_, kSearch.y_, kSearch.width_,
                                     kSearch.height_, style_.border_);
    const float kTextY = kSearch.y_ + (kSearch.height_ - kFont) / 2.0F;
    const float kTextX = kSearch.x_ + style_.item_padding_ / 2.0F;
    if (query_.empty()) {
        utils::DrawTextWrapped(kPlaceholder, kTextX, kTextY, style_.font_size_,
                               style_.disabled_text_);
    } else {
        utils::DrawTextWrapped(query_.c_str(), kTextX, kTextY,
                               style_.font_size_, style_.text_);
    }
    const float kCaretX =
        kTextX + (query_.empty() ? 0.0F : measurer_(query_, style_.font_size_));
    utils::DrawRectangleWrapped(kCaretX + 1.0F, kTextY, 1.0F, kFont,
                                style_.text_);

    // Rows.
    for (std::size_t row = scroll_;
         row < rows_.size() && row < scroll_ + visible_rows_; ++row) {
        const auto kRect = RowRect(row);
        if (!kRect) {
            continue;
        }
        const Row &current = rows_[row];
        const float kRowTextY = kRect->y_ + (kRect->height_ - kFont) / 2.0F;
        const float kLabelX = kRect->x_ + TextX(current.depth_);

        if (selected_ == row && current.kind_ != RowKind::kEmpty) {
            utils::DrawRectangleWrapped(kRect->x_ + 1.0F, kRect->y_,
                                        kRect->width_ - 2.0F, kRect->height_,
                                        style_.hover_);
        }
        switch (current.kind_) {
            case RowKind::kFolder:
                utils::DrawTextWrapped(current.expanded_ ? "v" : ">",
                                       kLabelX - kArrowWidth, kRowTextY,
                                       style_.font_size_, kFolderText);
                utils::DrawTextWrapped(current.label_.c_str(), kLabelX,
                                       kRowTextY, style_.font_size_,
                                       kFolderText);
                break;
            case RowKind::kEntry:
                utils::DrawTextWrapped(current.label_.c_str(), kLabelX,
                                       kRowTextY, style_.font_size_,
                                       style_.text_);
                break;
            case RowKind::kEmpty:
                utils::DrawTextWrapped(current.label_.c_str(), kLabelX,
                                       kRowTextY, style_.font_size_,
                                       style_.disabled_text_);
                break;
        }
    }

    // Scrollbar, when the rows do not fit.
    if (rows_.size() > visible_rows_) {
        const auto kFirst = RowRect(scroll_);
        if (kFirst) {
            const float kTrack =
                static_cast<float>(visible_rows_) * style_.item_height_;
            const float kRatio = static_cast<float>(visible_rows_) /
                                 static_cast<float>(rows_.size());
            const float kOffset = static_cast<float>(scroll_) /
                                  static_cast<float>(rows_.size()) * kTrack;
            utils::DrawRectangleWrapped(
                kBounds.x_ + kBounds.width_ - kScrollbarWidth - 1.0F,
                kFirst->y_ + kOffset, kScrollbarWidth - 1.0F,
                std::max(kTrack * kRatio, 8.0F), style_.border_);
        }
    }
}

}  // namespace editor::ui
