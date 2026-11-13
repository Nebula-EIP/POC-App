/**
 * @file component_adapter.cpp
 * @brief Draws the components a module describes for a node and turns user
 * input on them into the components' callbacks.
 *
 * @author Created by Nolan Papa
 * @date Created on 20-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 13-11-2026
 */

#include "component_adapter.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <type_traits>
#include <variant>

namespace {

using core::capa::Button;
using core::capa::Checkbox;
using core::capa::Component;
using core::capa::ComponentList;
using core::capa::InfoBubble;
using core::capa::Label;
using core::capa::NumberField;
using core::capa::Select;
using core::capa::Separator;
using core::capa::Slider;
using core::capa::TextField;
using utils::WrappedColor;
using utils::WrappedRectangle;

constexpr int kFontSize = 16;
constexpr float kSpacing = 2.0F;
constexpr float kCharWidth = 8.0F;
constexpr float kLabelGap = 12.0F;
constexpr float kMinControlWidth = 96.0F;
constexpr float kControlPadding = 6.0F;
constexpr float kRowGap = 4.0F;
constexpr float kBubbleHeight = 22.0F;
/// Text drawn in a field starts this far below the top of its box.
constexpr float kTextTop = 2.0F;
/// The caret and the selection leave this much of the box above and below.
constexpr float kCaretMargin = 2.0F;
/// Caret width in screen pixels (at least one world unit).
constexpr float kCaretPixels = 1.5F;
/// The caret blinks: visible, then hidden, for this long each (seconds).
constexpr double kBlinkHalfPeriod = 0.53;

constexpr WrappedColor kText = {245, 247, 250, 255};
constexpr WrappedColor kMutedText = {185, 193, 204, 255};
constexpr WrappedColor kDisabledText = {120, 126, 136, 255};
constexpr WrappedColor kWarningText = {255, 214, 72, 255};
constexpr WrappedColor kFieldIdle = {50, 55, 65, 255};
constexpr WrappedColor kField = {62, 69, 82, 255};
constexpr WrappedColor kFieldEditing = {74, 82, 98, 255};
constexpr WrappedColor kAccent = {100, 160, 255, 255};
constexpr WrappedColor kError = {230, 80, 80, 255};
constexpr WrappedColor kErrorBubble = {120, 30, 30, 240};
constexpr WrappedColor kListBackground = {35, 40, 48, 250};
constexpr WrappedColor kListSelected = {70, 100, 150, 255};
constexpr WrappedColor kSeparator = {90, 96, 108, 255};
constexpr WrappedColor kSelection = {100, 160, 255, 110};

/**
 * @brief Number of characters (UTF-8 code points) of a text.
 *
 */
std::size_t CodePoints(const std::string &text) {
    return static_cast<std::size_t>(
        std::count_if(text.begin(), text.end(), [](char character) {
            return (static_cast<unsigned char>(character) & 0xC0U) != 0x80U;
        }));
}

float TextWidth(const std::string &text) {
    return static_cast<float>(CodePoints(text)) * kCharWidth;
}

void PopCodePoint(std::string &text) {
    while (!text.empty()) {
        const auto kByte = static_cast<unsigned char>(text.back());
        text.pop_back();
        if ((kByte & 0xC0U) != 0x80U) {
            return;
        }
    }
}

void AppendCodePoint(std::string &text, int code_point) {
    const auto kValue = static_cast<unsigned int>(code_point);
    if (kValue < 0x80U) {
        text.push_back(static_cast<char>(kValue));
    } else if (kValue < 0x800U) {
        text.push_back(static_cast<char>(0xC0U | (kValue >> 6U)));
        text.push_back(static_cast<char>(0x80U | (kValue & 0x3FU)));
    } else if (kValue < 0x10000U) {
        text.push_back(static_cast<char>(0xE0U | (kValue >> 12U)));
        text.push_back(static_cast<char>(0x80U | ((kValue >> 6U) & 0x3FU)));
        text.push_back(static_cast<char>(0x80U | (kValue & 0x3FU)));
    } else {
        text.push_back(static_cast<char>(0xF0U | (kValue >> 18U)));
        text.push_back(static_cast<char>(0x80U | ((kValue >> 12U) & 0x3FU)));
        text.push_back(static_cast<char>(0x80U | ((kValue >> 6U) & 0x3FU)));
        text.push_back(static_cast<char>(0x80U | (kValue & 0x3FU)));
    }
}

/**
 * @brief Shortens a text to a width (estimated), keeping its start.
 *
 */
std::string Fit(const std::string &text, float width) {
    const auto kMax =
        static_cast<std::size_t>(std::max(width, 0.0F) / kCharWidth);
    std::string result = text;
    while (CodePoints(result) > kMax && !result.empty()) {
        PopCodePoint(result);
    }
    return result;
}

bool IsLeadByte(char byte) {
    return (static_cast<unsigned char>(byte) & 0xC0U) != 0x80U;
}

/**
 * @brief Byte offset of a character (UTF-8 code point) index; the text size
 * past the last character.
 *
 */
std::size_t ByteOffset(const std::string &text, std::size_t index) {
    std::size_t count = 0;
    for (std::size_t byte = 0; byte < text.size(); ++byte) {
        if (IsLeadByte(text[byte])) {
            if (count == index) {
                return byte;
            }
            ++count;
        }
    }
    return text.size();
}

/**
 * @brief Characters [first, last) of a text.
 *
 */
std::string Characters(const std::string &text, std::size_t first,
                       std::size_t last) {
    const std::size_t kBegin = ByteOffset(text, first);
    const std::size_t kEnd = ByteOffset(text, last);
    return kEnd > kBegin ? text.substr(kBegin, kEnd - kBegin) : std::string{};
}

/**
 * @brief Letters, digits, '_' and any non-ASCII character belong to words.
 *
 */
bool IsWordCharacter(const std::string &text, std::size_t index) {
    const auto kByte =
        static_cast<unsigned char>(text[ByteOffset(text, index)]);
    return kByte >= 0x80U || std::isalnum(kByte) != 0 || kByte == '_';
}

/**
 * @brief Start of the word before a position (Ctrl+Left).
 *
 */
std::size_t PreviousWord(const std::string &text, std::size_t position) {
    while (position > 0 && !IsWordCharacter(text, position - 1)) {
        --position;
    }
    while (position > 0 && IsWordCharacter(text, position - 1)) {
        --position;
    }
    return position;
}

/**
 * @brief End of the word after a position (Ctrl+Right).
 *
 */
std::size_t NextWord(const std::string &text, std::size_t position) {
    const std::size_t kCount = CodePoints(text);
    while (position < kCount && !IsWordCharacter(text, position)) {
        ++position;
    }
    while (position < kCount && IsWordCharacter(text, position)) {
        ++position;
    }
    return position;
}

/**
 * @brief Removes line breaks and other control characters: fields hold one
 * line.
 *
 */
std::string SingleLine(const std::string &text) {
    std::string result;
    result.reserve(text.size());
    for (const char kByte : text) {
        const auto kValue = static_cast<unsigned char>(kByte);
        if (kValue >= 0x20U && kValue != 0x7FU) {
            result.push_back(kByte);
        }
    }
    return result;
}

core::capa::ComponentId IdOf(const Component &component) {
    return std::visit([](const auto &value) { return value.id_; }, component);
}

/**
 * @brief Label drawn in the left column, or nullptr for full-width components.
 *
 */
const std::string *LabelOf(const Component &component) {
    return std::visit(
        [](const auto &value) -> const std::string * {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, TextField> ||
                          std::is_same_v<T, NumberField> ||
                          std::is_same_v<T, Slider> ||
                          std::is_same_v<T, Checkbox> ||
                          std::is_same_v<T, Select>) {
                return &value.label_;
            } else {
                return nullptr;
            }
        },
        component);
}

float LabelColumn(const ComponentList &components) {
    float width = 0.0F;
    for (const Component &component : components) {
        if (const std::string *label = LabelOf(component)) {
            width = std::max(width, TextWidth(*label) + kLabelGap);
        }
    }
    return width;
}

/**
 * @brief Width the content of a component needs (without its label).
 *
 */
float ContentWidth(const Component &component) {
    return std::visit(
        [](const auto &value) -> float {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, Label> ||
                          std::is_same_v<T, InfoBubble>) {
                return TextWidth(value.text_);
            } else if constexpr (std::is_same_v<T, Button>) {
                return TextWidth(value.label_) + 2.0F * kControlPadding;
            } else if constexpr (std::is_same_v<T, TextField>) {
                return TextWidth(value.value_) + 2.0F * kControlPadding;
            } else if constexpr (std::is_same_v<T, NumberField> ||
                                 std::is_same_v<T, Slider>) {
                return TextWidth(editor::ui::ComponentAdapter::FormatNumber(
                           value.value_, value.is_integer_)) +
                       2.0F * kControlPadding;
            } else if constexpr (std::is_same_v<T, Select>) {
                float width = 0.0F;
                for (const auto &option : value.options_) {
                    width = std::max(width, TextWidth(option));
                }
                return width + 3.0F * kControlPadding + 10.0F;
            } else {
                return 0.0F;
            }
        },
        component);
}

bool Contains(WrappedRectangle rectangle, utils::WrappedVector2 point) {
    return point.x_ >= rectangle.x_ &&
           point.x_ <= rectangle.x_ + rectangle.width_ &&
           point.y_ >= rectangle.y_ &&
           point.y_ <= rectangle.y_ + rectangle.height_;
}

WrappedRectangle RowBounds(WrappedRectangle bounds, std::size_t index) {
    return {bounds.x_,
            bounds.y_ + static_cast<float>(index) *
                            editor::ui::ComponentAdapter::kRowHeight,
            bounds.width_, editor::ui::ComponentAdapter::kRowHeight};
}

std::optional<std::size_t> IndexOfId(const ComponentList &components,
                                     core::capa::ComponentId id) {
    for (std::size_t index = 0; index < components.size(); ++index) {
        if (IdOf(components[index]) == id) {
            return index;
        }
    }
    return std::nullopt;
}

void DrawBox(WrappedRectangle box, WrappedColor color) {
    utils::DrawRectangleWrapped(box.x_, box.y_, box.width_, box.height_, color);
}

}  // namespace

namespace editor::ui {

ComponentInput ComponentInput::Read(utils::WrappedVector2 cursor) {
    using utils::WrappedKey;
    const auto kPressed = [](WrappedKey key) {
        return utils::IsKeyPressedWrapped(key);
    };
    const auto kRepeated = [](WrappedKey key) {
        return utils::IsKeyPressedOrRepeatWrapped(key);
    };

    ComponentInput input;
    input.cursor_ = cursor;
    input.left_pressed_ = utils::IsLeftClicked();
    input.left_down_ = utils::IsLeftDown();
    input.double_click_ = utils::IsLeftDoubleClicked();
    for (int character = utils::GetCharPressedWrapped(); character > 0;
         character = utils::GetCharPressedWrapped()) {
        AppendCodePoint(input.typed_, character);
    }
    input.backspace_ = kRepeated(WrappedKey::kBackspace);
    input.delete_ = kRepeated(WrappedKey::kDelete);
    input.enter_ =
        kPressed(WrappedKey::kEnter) || kPressed(WrappedKey::kKpEnter);
    input.escape_ = kPressed(WrappedKey::kEscape);
    input.left_ = kRepeated(WrappedKey::kLeft);
    input.right_ = kRepeated(WrappedKey::kRight);
    input.home_ = kPressed(WrappedKey::kHome);
    input.end_ = kPressed(WrappedKey::kEnd);
    input.shift_ = utils::IsShiftDownWrapped();
    // AltGr is reported as Ctrl + Alt: it types characters ('@', '#'...),
    // it is not a shortcut.
    input.control_ =
        utils::IsControlDownWrapped() && !utils::IsAltDownWrapped();
    // Keys are physical QWERTY keys: AZERTY 'A' is reported as 'Q'.
    input.select_all_ = input.control_ &&
                        (kPressed(WrappedKey::kA) || kPressed(WrappedKey::kQ));
    input.copy_ = input.control_ && kPressed(WrappedKey::kC);
    input.cut_ = input.control_ && kPressed(WrappedKey::kX);
    input.paste_ = input.control_ && kPressed(WrappedKey::kV);
    input.time_ = utils::GetTimeWrapped();
    return input;
}

ComponentAdapter::ComponentAdapter(TextMeasurer measurer, Clipboard clipboard)
    : measurer_(std::move(measurer)), clipboard_(std::move(clipboard)) {
    // Without a window (tests), the font and the clipboard are unavailable:
    // texts are measured like the layout does, the clipboard is empty.
    if (!measurer_) {
        measurer_ = [](const std::string &text) {
            return utils::IsRaylibReady()
                       ? utils::MeasureTextExWrapped(text.c_str(), kFontSize,
                                                     kSpacing)
                       : TextWidth(text);
        };
    }
    if (!clipboard_.read_) {
        clipboard_.read_ = [] {
            return utils::IsRaylibReady() ? utils::GetClipboardTextWrapped()
                                          : std::string{};
        };
    }
    if (!clipboard_.write_) {
        clipboard_.write_ = [](const std::string &text) {
            if (utils::IsRaylibReady()) utils::SetClipboardTextWrapped(text);
        };
    }
}

utils::WrappedVector2 ComponentAdapter::PreferredSize(
    const ComponentList &components) {
    if (components.empty()) {
        return {0.0F, 0.0F};
    }
    const float kLabels = LabelColumn(components);
    float width = 0.0F;
    for (const Component &component : components) {
        const float kContent = ContentWidth(component);
        width =
            std::max(width, LabelOf(component) != nullptr
                                ? kLabels + std::max(kMinControlWidth, kContent)
                                : kContent);
    }
    return {width, static_cast<float>(components.size()) * kRowHeight};
}

WrappedRectangle ComponentAdapter::ControlBounds(
    const ComponentList &components, WrappedRectangle bounds,
    std::size_t index) {
    const WrappedRectangle kRow = RowBounds(bounds, index);
    const float kLabels =
        index < components.size() && LabelOf(components[index]) != nullptr
            ? LabelColumn(components)
            : 0.0F;
    return {kRow.x_ + kLabels, kRow.y_ + kRowGap / 2.0F,
            std::max(kRow.width_ - kLabels, 0.0F), kRow.height_ - kRowGap};
}

WrappedRectangle ComponentAdapter::OptionBounds(const ComponentList &components,
                                                WrappedRectangle bounds,
                                                std::size_t index,
                                                std::size_t option) {
    const WrappedRectangle kControl = ControlBounds(components, bounds, index);
    return {kControl.x_,
            kControl.y_ + kControl.height_ +
                static_cast<float>(option) * kControl.height_,
            kControl.width_, kControl.height_};
}

std::string ComponentAdapter::FormatNumber(double value, bool integer) {
    char text[64];
    std::snprintf(text, sizeof(text), integer ? "%.0f" : "%g", value);
    return text;
}

std::string ComponentAdapter::ValidateNumber(const std::string &text,
                                             const NumberField &field,
                                             double *value) {
    std::string normalized;
    for (const char kCharacter : text) {
        if (kCharacter != ' ') {
            normalized.push_back(kCharacter == ',' ? '.' : kCharacter);
        }
    }
    if (normalized.empty()) {
        return "Entrez un nombre.";
    }
    char *end = nullptr;
    const double kParsed = std::strtod(normalized.c_str(), &end);
    if (end == normalized.c_str() || *end != '\0' || !std::isfinite(kParsed)) {
        return "« " + text + " » n'est pas un nombre.";
    }
    if (field.is_integer_ && kParsed != std::floor(kParsed)) {
        return "La valeur doit être un nombre entier.";
    }
    if (kParsed < field.min_ || kParsed > field.max_) {
        return "La valeur doit être comprise entre " +
               FormatNumber(field.min_, field.is_integer_) + " et " +
               FormatNumber(field.max_, field.is_integer_) + ".";
    }
    if (value != nullptr) {
        *value = kParsed;
    }
    return {};
}

bool ComponentAdapter::IsEditingText() const noexcept {
    return edit_.has_value();
}

bool ComponentAdapter::IsListOpen() const noexcept {
    return open_list_.has_value();
}

std::optional<core::NodeId> ComponentAdapter::FocusedNode() const noexcept {
    if (edit_) return edit_->node_id_;
    if (open_list_) return open_list_->node_id_;
    return std::nullopt;
}

const std::string &ComponentAdapter::EditedText() const noexcept {
    static const std::string kEmpty;
    return edit_ ? edit_->text_ : kEmpty;
}

const std::string &ComponentAdapter::EditError() const noexcept {
    static const std::string kEmpty;
    return edit_ ? edit_->error_ : kEmpty;
}

std::size_t ComponentAdapter::Caret() const noexcept {
    return edit_ ? edit_->caret_ : 0;
}

std::pair<std::size_t, std::size_t> ComponentAdapter::Selection()
    const noexcept {
    if (!edit_) return {0, 0};
    return {std::min(edit_->anchor_, edit_->caret_),
            std::max(edit_->anchor_, edit_->caret_)};
}

std::string ComponentAdapter::SelectedText() const {
    if (!edit_) return {};
    const auto [kFirst, kLast] = Selection();
    return Characters(edit_->text_, kFirst, kLast);
}

std::size_t ComponentAdapter::FirstVisibleCharacter() const noexcept {
    return edit_ ? edit_->scroll_ : 0;
}

void ComponentAdapter::CancelEdit() noexcept {
    edit_.reset();
    open_list_.reset();
}

std::vector<std::string> ComponentAdapter::TakeMessages() {
    std::vector<std::string> messages = std::move(messages_);
    messages_.clear();
    return messages;
}

void ComponentAdapter::StartEdit(core::NodeId node_id,
                                 const Component &component) {
    Edit edit;
    edit.node_id_ = node_id;
    edit.component_id_ = IdOf(component);
    if (const auto *field = std::get_if<TextField>(&component)) {
        edit.text_ = field->value_;
    } else if (const auto *number = std::get_if<NumberField>(&component)) {
        edit.text_ = FormatNumber(number->value_, number->is_integer_);
    }
    // The caret starts at the end; a click then moves it where it was.
    edit.caret_ = CodePoints(edit.text_);
    edit.anchor_ = edit.caret_;
    edit.caret_time_ = now_;
    edit_ = std::move(edit);
    UpdateEditError(component);
}

void ComponentAdapter::UpdateEditError(const Component &component) {
    if (!edit_) return;
    edit_->error_.clear();
    if (const auto *field = std::get_if<TextField>(&component)) {
        if (field->validate_) {
            edit_->error_ = field->validate_(edit_->text_);
        }
    } else if (const auto *number = std::get_if<NumberField>(&component)) {
        edit_->error_ = ValidateNumber(edit_->text_, *number, nullptr);
    }
}

void ComponentAdapter::FinishEdit(const ComponentList &components, bool apply) {
    if (!edit_) return;
    // Reset first: the callbacks may lead to a new frame of components.
    const Edit kEdit = std::move(*edit_);
    edit_.reset();
    if (!apply) return;

    const auto kIndex = IndexOfId(components, kEdit.component_id_);
    if (!kIndex) return;
    if (!kEdit.error_.empty()) {
        messages_.push_back("Modification annulée : " + kEdit.error_);
        return;
    }
    const Component &component = components[*kIndex];
    if (const auto *field = std::get_if<TextField>(&component)) {
        if (kEdit.text_ != field->value_ && field->on_change_) {
            field->on_change_(kEdit.text_);
        }
    } else if (const auto *number = std::get_if<NumberField>(&component)) {
        double value = number->value_;
        if (ValidateNumber(kEdit.text_, *number, &value).empty() &&
            value != number->value_ && number->on_change_) {
            number->on_change_(value);
        }
    }
}

bool ComponentAdapter::HandleOpenList(const ComponentList &components,
                                      WrappedRectangle bounds,
                                      const ComponentInput &input) {
    const auto kIndex = IndexOfId(components, open_list_->component_id_);
    const Select *select =
        kIndex ? std::get_if<Select>(&components[*kIndex]) : nullptr;
    if (select == nullptr || !select->enabled_) {
        open_list_.reset();
        return false;
    }
    if (input.escape_) {
        open_list_.reset();
        return false;
    }
    if (!input.left_pressed_) {
        return false;
    }
    for (std::size_t option = 0; option < select->options_.size(); ++option) {
        if (Contains(OptionBounds(components, bounds, *kIndex, option),
                     input.cursor_)) {
            open_list_.reset();
            if (option != select->selected_index_ && select->on_change_) {
                select->on_change_(option);
            }
            return true;
        }
    }
    // Any other click closes the list; a click on the select itself is used.
    open_list_.reset();
    return Contains(RowBounds(bounds, *kIndex), input.cursor_);
}

bool ComponentAdapter::Update(core::NodeId node_id,
                              const ComponentList &components,
                              WrappedRectangle bounds,
                              const ComponentInput &input) {
    now_ = input.time_;
    // The edited node changed without a click on the fields: drop the edit.
    if (edit_ && edit_->node_id_ != node_id) edit_.reset();
    if (open_list_ && open_list_->node_id_ != node_id) open_list_.reset();

    if (open_list_ && HandleOpenList(components, bounds, input)) {
        return true;
    }

    bool captured = false;
    if (edit_) {
        const auto kIndex = IndexOfId(components, edit_->component_id_);
        if (!kIndex) {
            edit_.reset();
        } else {
            captured = HandleEditInput(components, bounds, *kIndex, input);
        }
    }
    if (input.left_pressed_) {
        captured = HandleClick(node_id, components, bounds, input);
    }
    KeepCaretVisible(components, bounds);
    return captured;
}

bool ComponentAdapter::HandleEditInput(const ComponentList &components,
                                       WrappedRectangle bounds,
                                       std::size_t index,
                                       const ComponentInput &input) {
    // A press in the field started a mouse selection: it follows the cursor
    // until the button is released, and the pointer stays with the field.
    bool selecting = false;
    if (edit_->dragging_) {
        if (!input.left_down_) {
            edit_->dragging_ = false;
        } else if (!input.left_pressed_) {
            MoveCaret(IndexAt(ControlBounds(components, bounds, index),
                              input.cursor_, true),
                      true);
            selecting = true;
        }
    }

    if (EditText(input)) {
        UpdateEditError(components[index]);
    }
    if (input.escape_) {
        FinishEdit(components, false);
    } else if (input.enter_ && edit_->error_.empty()) {
        FinishEdit(components, true);
    }
    return selecting;
}

bool ComponentAdapter::EditText(const ComponentInput &input) {
    Edit &edit = *edit_;
    const std::size_t kCaret = edit.caret_;
    const std::size_t kAnchor = edit.anchor_;
    bool changed = false;

    if (input.select_all_) {
        edit.anchor_ = 0;
        edit.caret_ = CodePoints(edit.text_);
    }
    if ((input.copy_ || input.cut_) && edit.anchor_ != edit.caret_ &&
        clipboard_.write_) {
        clipboard_.write_(SelectedText());
    }
    if (input.cut_) {
        changed = EraseSelection() || changed;
    }
    if (input.paste_ && clipboard_.read_) {
        const std::string kPasted = SingleLine(clipboard_.read_());
        if (!kPasted.empty()) {
            Insert(kPasted);
            changed = true;
        }
    }
    // Some systems also type the letter of a Ctrl shortcut: drop it.
    const bool kShortcut =
        input.select_all_ || input.copy_ || input.cut_ || input.paste_;
    if (!kShortcut && !input.typed_.empty()) {
        const std::string kTyped = SingleLine(input.typed_);
        if (!kTyped.empty()) {
            Insert(kTyped);
            changed = true;
        }
    }

    // Backspace and Delete erase the selection, else one character (one
    // word with Ctrl) before or after the caret.
    if (input.backspace_) {
        if (EraseSelection()) {
            changed = true;
        } else if (edit.caret_ > 0) {
            EraseRange(input.control_ ? PreviousWord(edit.text_, edit.caret_)
                                      : edit.caret_ - 1,
                       edit.caret_);
            changed = true;
        }
    }
    if (input.delete_) {
        if (EraseSelection()) {
            changed = true;
        } else if (edit.caret_ < CodePoints(edit.text_)) {
            EraseRange(edit.caret_, input.control_
                                        ? NextWord(edit.text_, edit.caret_)
                                        : edit.caret_ + 1);
            changed = true;
        }
    }

    const bool kSelected = edit.anchor_ != edit.caret_;
    if (input.left_) {
        std::size_t target = edit.caret_ > 0 ? edit.caret_ - 1 : 0;
        if (input.control_) {
            target = PreviousWord(edit.text_, edit.caret_);
        } else if (kSelected && !input.shift_) {
            target = std::min(edit.anchor_, edit.caret_);
        }
        MoveCaret(target, input.shift_);
    }
    if (input.right_) {
        const std::size_t kCount = CodePoints(edit.text_);
        std::size_t target = std::min(edit.caret_ + 1, kCount);
        if (input.control_) {
            target = NextWord(edit.text_, edit.caret_);
        } else if (kSelected && !input.shift_) {
            target = std::max(edit.anchor_, edit.caret_);
        }
        MoveCaret(target, input.shift_);
    }
    if (input.home_) {
        MoveCaret(0, input.shift_);
    }
    if (input.end_) {
        MoveCaret(CodePoints(edit.text_), input.shift_);
    }

    if (changed || edit.caret_ != kCaret || edit.anchor_ != kAnchor) {
        edit.caret_time_ = now_;
    }
    return changed;
}

void ComponentAdapter::Insert(const std::string &text) {
    EraseSelection();
    Edit &edit = *edit_;
    edit.text_.insert(ByteOffset(edit.text_, edit.caret_), text);
    edit.caret_ += CodePoints(text);
    edit.anchor_ = edit.caret_;
}

void ComponentAdapter::EraseRange(std::size_t first, std::size_t last) {
    Edit &edit = *edit_;
    const std::size_t kBegin = ByteOffset(edit.text_, first);
    const std::size_t kEnd = ByteOffset(edit.text_, last);
    if (kEnd > kBegin) {
        edit.text_.erase(kBegin, kEnd - kBegin);
    }
    edit.caret_ = first;
    edit.anchor_ = first;
}

bool ComponentAdapter::EraseSelection() {
    const auto [kFirst, kLast] = Selection();
    if (kFirst == kLast) {
        return false;
    }
    EraseRange(kFirst, kLast);
    return true;
}

void ComponentAdapter::MoveCaret(std::size_t position, bool extend) {
    Edit &edit = *edit_;
    edit.caret_ = std::min(position, CodePoints(edit.text_));
    if (!extend) {
        edit.anchor_ = edit.caret_;
    }
    edit.caret_time_ = now_;
}

bool ComponentAdapter::HandleClick(core::NodeId node_id,
                                   const ComponentList &components,
                                   WrappedRectangle bounds,
                                   const ComponentInput &input) {
    for (std::size_t index = 0; index < components.size(); ++index) {
        if (!Contains(RowBounds(bounds, index), input.cursor_)) {
            continue;
        }
        const Component &component = components[index];
        if (edit_ && edit_->component_id_ != IdOf(component)) {
            FinishEdit(components, true);
        }
        std::visit(
            [&](const auto &value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, TextField>) {
                    if (value.enabled_ && !value.read_only_) {
                        ClickField(node_id, components, bounds, index, input);
                    }
                } else if constexpr (std::is_same_v<T, NumberField>) {
                    if (value.enabled_) {
                        ClickField(node_id, components, bounds, index, input);
                    }
                } else if constexpr (std::is_same_v<T, Checkbox>) {
                    if (value.enabled_ && value.on_change_) {
                        value.on_change_(!value.value_);
                    }
                } else if constexpr (std::is_same_v<T, Select>) {
                    if (value.enabled_ && !value.options_.empty()) {
                        open_list_ = OpenList{node_id, value.id_};
                    }
                } else if constexpr (std::is_same_v<T, Button>) {
                    if (value.enabled_ && value.on_click_) {
                        value.on_click_();
                    }
                } else if constexpr (std::is_same_v<T, Slider>) {
                    if (value.enabled_ && value.max_ > value.min_ &&
                        value.on_change_) {
                        const WrappedRectangle kControl =
                            ControlBounds(components, bounds, index);
                        const double kRatio =
                            std::clamp((input.cursor_.x_ - kControl.x_) /
                                           std::max(kControl.width_, 1.0F),
                                       0.0F, 1.0F);
                        double next =
                            value.min_ + kRatio * (value.max_ - value.min_);
                        if (value.step_ > 0.0) {
                            next = value.min_ + std::round((next - value.min_) /
                                                           value.step_) *
                                                    value.step_;
                        }
                        if (value.is_integer_) next = std::round(next);
                        value.on_change_(
                            std::clamp(next, value.min_, value.max_));
                    }
                }
            },
            component);
        return true;
    }

    // A click outside the fields applies the edit (or drops it if invalid).
    FinishEdit(components, true);
    return false;
}

void ComponentAdapter::ClickField(core::NodeId node_id,
                                  const ComponentList &components,
                                  WrappedRectangle bounds, std::size_t index,
                                  const ComponentInput &input) {
    if (!edit_) {
        StartEdit(node_id, components[index]);
    }
    Edit &edit = *edit_;
    if (input.double_click_) {
        edit.anchor_ = 0;
        edit.caret_ = CodePoints(edit.text_);
        edit.dragging_ = false;
        edit.caret_time_ = now_;
        return;
    }
    const WrappedRectangle kControl = ControlBounds(components, bounds, index);
    if (!Contains(kControl, input.cursor_)) {
        // On the label: the field gets the keyboard, the caret stays.
        return;
    }
    MoveCaret(IndexAt(kControl, input.cursor_, false), input.shift_);
    edit.dragging_ = true;
}

void ComponentAdapter::KeepCaretVisible(const ComponentList &components,
                                        WrappedRectangle bounds) {
    if (!edit_) return;
    const auto kIndex = IndexOfId(components, edit_->component_id_);
    if (!kIndex) return;
    const float kWidth = ControlBounds(components, bounds, *kIndex).width_ -
                         2.0F * kControlPadding;
    Edit &edit = *edit_;
    if (edit.caret_ < edit.scroll_) {
        edit.scroll_ = edit.caret_;
    }
    while (edit.scroll_ < edit.caret_ &&
           CaretOffset(edit.text_, edit.scroll_, edit.caret_) > kWidth) {
        ++edit.scroll_;
    }
    // Room left on the right (text erased): show more of the start.
    const std::size_t kCount = CodePoints(edit.text_);
    while (edit.scroll_ > 0 &&
           CaretOffset(edit.text_, edit.scroll_ - 1, kCount) <= kWidth) {
        --edit.scroll_;
    }
}

float ComponentAdapter::Span(const std::string &text, std::size_t first,
                             std::size_t last) const {
    return last > first ? measurer_(Characters(text, first, last)) : 0.0F;
}

float ComponentAdapter::CaretOffset(const std::string &text, std::size_t first,
                                    std::size_t last) const {
    // The caret sits in the middle of the spacing between two characters.
    return last > first ? Span(text, first, last) + kSpacing / 2.0F : 0.0F;
}

std::size_t ComponentAdapter::LastVisible(const std::string &text,
                                          std::size_t first,
                                          float width) const {
    const std::size_t kCount = CodePoints(text);
    std::size_t last = first;
    while (last < kCount && Span(text, first, last + 1) <= width) {
        ++last;
    }
    return last;
}

std::size_t ComponentAdapter::IndexAt(WrappedRectangle control,
                                      utils::WrappedVector2 point,
                                      bool dragging) const {
    const Edit &edit = *edit_;
    const float kWidth = control.width_ - 2.0F * kControlPadding;
    const float kLocal = point.x_ - (control.x_ + kControlPadding);
    const std::size_t kLast = LastVisible(edit.text_, edit.scroll_, kWidth);
    // While selecting, going past an edge scrolls one character per frame.
    if (kLocal < 0.0F) {
        return dragging && edit.scroll_ > 0 ? edit.scroll_ - 1 : edit.scroll_;
    }
    if (kLocal > kWidth && dragging) {
        return std::min(kLast + 1, CodePoints(edit.text_));
    }
    // Closest boundary between two characters.
    float previous = 0.0F;
    for (std::size_t index = edit.scroll_ + 1; index <= kLast; ++index) {
        const float kOffset = CaretOffset(edit.text_, edit.scroll_, index);
        if (kOffset >= kLocal) {
            return kOffset - kLocal < kLocal - previous ? index : index - 1;
        }
        previous = kOffset;
    }
    return kLast;
}

void ComponentAdapter::DrawValue(WrappedRectangle box, const std::string &text,
                                 WrappedColor color) const {
    const std::string kShown = Characters(
        text, 0, LastVisible(text, 0, box.width_ - 2.0F * kControlPadding));
    utils::DrawTextExWrapped(kShown.c_str(), box.x_ + kControlPadding,
                             box.y_ + kTextTop, kFontSize, kSpacing, color);
}

void ComponentAdapter::DrawEditedText(WrappedRectangle control,
                                      float pixel_size) const {
    const Edit &edit = *edit_;
    const float kLeft = control.x_ + kControlPadding;
    const float kTop = control.y_ + kCaretMargin;
    const float kHeight = control.height_ - 2.0F * kCaretMargin;
    const std::size_t kLast = LastVisible(
        edit.text_, edit.scroll_, control.width_ - 2.0F * kControlPadding);

    // Only the visible part of the selection is highlighted.
    const auto [kFirst, kEnd] = Selection();
    const std::size_t kFrom = std::clamp(kFirst, edit.scroll_, kLast);
    const std::size_t kTo = std::clamp(kEnd, edit.scroll_, kLast);
    if (kFrom < kTo) {
        const float kX = kLeft + CaretOffset(edit.text_, edit.scroll_, kFrom);
        utils::DrawRectangleRecWrapped(
            {kX, kTop, kLeft + CaretOffset(edit.text_, edit.scroll_, kTo) - kX,
             kHeight},
            kSelection);
    }

    utils::DrawTextExWrapped(
        Characters(edit.text_, edit.scroll_, kLast).c_str(), kLeft,
        control.y_ + kTextTop, kFontSize, kSpacing, kText);

    // The caret blinks, and shows right away when it moves.
    const double kElapsed = std::max(now_ - edit.caret_time_, 0.0);
    if (std::fmod(kElapsed, 2.0 * kBlinkHalfPeriod) < kBlinkHalfPeriod) {
        const float kWidth = std::max(kCaretPixels * pixel_size, 1.0F);
        const float kX =
            kLeft + CaretOffset(edit.text_, edit.scroll_, edit.caret_);
        utils::DrawRectangleRecWrapped(
            {kX - kWidth / 2.0F, kTop, kWidth, kHeight}, kText);
    }
}

void ComponentAdapter::DrawRow(const ComponentList &components,
                               WrappedRectangle bounds, std::size_t index,
                               core::NodeId node_id, bool active,
                               float pixel_size) const {
    const Component &component = components[index];
    const WrappedRectangle kRow = RowBounds(bounds, index);
    const WrappedRectangle kControl = ControlBounds(components, bounds, index);
    const bool kEditing = edit_ && edit_->node_id_ == node_id &&
                          edit_->component_id_ == IdOf(component);
    const bool kListOpen = open_list_ && open_list_->node_id_ == node_id &&
                           open_list_->component_id_ == IdOf(component);

    if (const std::string *label = LabelOf(component)) {
        utils::DrawTextExWrapped(Fit(*label, kControl.x_ - kRow.x_).c_str(),
                                 kRow.x_, kRow.y_ + 4.0F, kFontSize, kSpacing,
                                 kMutedText);
    }

    std::visit(
        [&](const auto &value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, Label>) {
                utils::DrawTextExWrapped(Fit(value.text_, kRow.width_).c_str(),
                                         kRow.x_, kRow.y_ + 4.0F, kFontSize,
                                         kSpacing, kText);
            } else if constexpr (std::is_same_v<T, InfoBubble>) {
                utils::DrawTextExWrapped(value.text_.c_str(), kRow.x_,
                                         kRow.y_ + 4.0F, kFontSize, kSpacing,
                                         kWarningText);
            } else if constexpr (std::is_same_v<T, Separator>) {
                utils::DrawRectangleWrapped(kRow.x_,
                                            kRow.y_ + kRow.height_ / 2.0F,
                                            kRow.width_, 1.0F, kSeparator);
            } else {
                bool enabled = true;
                if constexpr (std::is_same_v<T, TextField>) {
                    enabled = value.enabled_ && !value.read_only_;
                } else {
                    enabled = value.enabled_;
                }
                const WrappedColor kBoxColor =
                    kEditing ? kFieldEditing
                             : (active && enabled ? kField : kFieldIdle);
                DrawBox(kControl, kBoxColor);
                const WrappedColor kValueColor =
                    enabled ? kText : kDisabledText;

                if constexpr (std::is_same_v<T, Button>) {
                    DrawValue(kControl, value.label_, kValueColor);
                } else if constexpr (std::is_same_v<T, TextField>) {
                    if (kEditing) {
                        DrawEditedText(kControl, pixel_size);
                    } else {
                        DrawValue(kControl, value.value_, kValueColor);
                    }
                } else if constexpr (std::is_same_v<T, NumberField>) {
                    if (kEditing) {
                        DrawEditedText(kControl, pixel_size);
                    } else {
                        DrawValue(kControl,
                                  FormatNumber(value.value_, value.is_integer_),
                                  kValueColor);
                    }
                } else if constexpr (std::is_same_v<T, Slider>) {
                    const double kRange = value.max_ - value.min_;
                    const float kRatio =
                        kRange > 0.0 ? static_cast<float>(std::clamp(
                                           (value.value_ - value.min_) / kRange,
                                           0.0, 1.0))
                                     : 0.0F;
                    DrawBox({kControl.x_, kControl.y_, kControl.width_ * kRatio,
                             kControl.height_},
                            kAccent);
                    DrawValue(kControl,
                              FormatNumber(value.value_, value.is_integer_),
                              kValueColor);
                } else if constexpr (std::is_same_v<T, Checkbox>) {
                    const float kSize = kControl.height_ - 6.0F;
                    const WrappedRectangle kSquare{
                        kControl.x_ + 4.0F, kControl.y_ + 3.0F, kSize, kSize};
                    utils::DrawRectangleLinesWrapped(
                        kSquare.x_, kSquare.y_, kSquare.width_, kSquare.height_,
                        kValueColor);
                    if (value.value_) {
                        DrawBox({kSquare.x_ + 3.0F, kSquare.y_ + 3.0F,
                                 kSquare.width_ - 6.0F, kSquare.height_ - 6.0F},
                                kAccent);
                    }
                    utils::DrawTextExWrapped(value.value_ ? "vrai" : "faux",
                                             kSquare.x_ + kSize + 6.0F,
                                             kControl.y_ + 2.0F, kFontSize,
                                             kSpacing, kValueColor);
                } else if constexpr (std::is_same_v<T, Select>) {
                    const std::string kSelected =
                        value.selected_index_ < value.options_.size()
                            ? value.options_[value.selected_index_]
                            : std::string{"<invalide>"};
                    DrawValue({kControl.x_, kControl.y_,
                               kControl.width_ - 14.0F, kControl.height_},
                              kSelected, kValueColor);
                    // Small chevron on the right of the box.
                    const float kX = kControl.x_ + kControl.width_ - 12.0F;
                    const float kY = kControl.y_ + kControl.height_ / 2.0F;
                    const float kDirection = kListOpen ? -1.0F : 1.0F;
                    utils::DrawLineWrapped({kX, kY - 2.0F * kDirection},
                                           {kX + 4.0F, kY + 2.0F * kDirection},
                                           1.5F, kValueColor);
                    utils::DrawLineWrapped({kX + 4.0F, kY + 2.0F * kDirection},
                                           {kX + 8.0F, kY - 2.0F * kDirection},
                                           1.5F, kValueColor);
                }

                if (kEditing || kListOpen) {
                    utils::DrawRectangleLinesWrapped(
                        kControl.x_, kControl.y_, kControl.width_,
                        kControl.height_,
                        kEditing && !edit_->error_.empty() ? kError : kAccent);
                }
            }
        },
        component);
}

void ComponentAdapter::DrawOverlay(core::NodeId node_id,
                                   const ComponentList &components,
                                   WrappedRectangle bounds) const {
    if (edit_ && edit_->node_id_ == node_id && !edit_->error_.empty()) {
        if (const auto kIndex = IndexOfId(components, edit_->component_id_)) {
            const WrappedRectangle kRow = RowBounds(bounds, *kIndex);
            const WrappedRectangle kBubble{
                kRow.x_, kRow.y_ + kRow.height_,
                std::max(kRow.width_,
                         TextWidth(edit_->error_) + 2.0F * kControlPadding),
                kBubbleHeight};
            DrawBox(kBubble, kErrorBubble);
            utils::DrawTextExWrapped(
                edit_->error_.c_str(), kBubble.x_ + kControlPadding,
                kBubble.y_ + 3.0F, kFontSize, kSpacing, kText);
        }
    }

    if (open_list_ && open_list_->node_id_ == node_id) {
        const auto kIndex = IndexOfId(components, open_list_->component_id_);
        const Select *select =
            kIndex ? std::get_if<Select>(&components[*kIndex]) : nullptr;
        if (select == nullptr) return;
        for (std::size_t option = 0; option < select->options_.size();
             ++option) {
            const WrappedRectangle kOption =
                OptionBounds(components, bounds, *kIndex, option);
            DrawBox(kOption, option == select->selected_index_
                                 ? kListSelected
                                 : kListBackground);
            DrawValue(kOption, select->options_[option], kText);
        }
        const WrappedRectangle kFirst =
            OptionBounds(components, bounds, *kIndex, 0);
        utils::DrawRectangleLinesWrapped(
            kFirst.x_, kFirst.y_, kFirst.width_,
            kFirst.height_ * static_cast<float>(select->options_.size()),
            kAccent);
    }
}

void ComponentAdapter::Draw(core::NodeId node_id,
                            const ComponentList &components,
                            WrappedRectangle bounds, bool active,
                            float pixel_size) const {
    for (std::size_t index = 0; index < components.size(); ++index) {
        DrawRow(components, bounds, index, node_id, active, pixel_size);
    }
    if (active) {
        DrawOverlay(node_id, components, bounds);
    }
}

void ComponentAdapter::ClearNode(core::NodeId node_id) noexcept {
    if (edit_ && edit_->node_id_ == node_id) edit_.reset();
    if (open_list_ && open_list_->node_id_ == node_id) open_list_.reset();
}

void ComponentAdapter::Clear() noexcept {
    edit_.reset();
    open_list_.reset();
    messages_.clear();
}

bool ComponentAdapter::HasState(core::NodeId node_id) const noexcept {
    return FocusedNode() == node_id;
}

}  // namespace editor::ui
