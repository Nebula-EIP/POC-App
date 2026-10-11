/**
 * @file component_adapter.cpp
 * @brief Draws the components a module describes for a node and turns user
 * input on them into the components' callbacks.
 *
 * @author Created by Nolan Papa
 * @date Created on 20-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 11-10-2026
 */

#include "component_adapter.hpp"

#include <algorithm>
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
constexpr float kCharWidth = 8.0F;
constexpr float kLabelGap = 12.0F;
constexpr float kMinControlWidth = 96.0F;
constexpr float kControlPadding = 6.0F;
constexpr float kRowGap = 4.0F;
constexpr float kBubbleHeight = 22.0F;

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

/// Number of characters (UTF-8 code points) of a text.
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

/// Shortens a text to a width: keeps the end when editing (caret visible).
std::string Fit(const std::string &text, float width, bool keep_end) {
    const auto kMax =
        static_cast<std::size_t>(std::max(width, 0.0F) / kCharWidth);
    if (CodePoints(text) <= kMax) {
        return text;
    }
    std::string result = text;
    if (keep_end) {
        while (CodePoints(result) > kMax && !result.empty()) {
            std::size_t length = 1;
            while (length < result.size() &&
                   (static_cast<unsigned char>(result[length]) & 0xC0U) ==
                       0x80U) {
                ++length;
            }
            result.erase(0, length);
        }
        return result;
    }
    while (CodePoints(result) > kMax && !result.empty()) {
        PopCodePoint(result);
    }
    return result;
}

core::capa::ComponentId IdOf(const Component &component) {
    return std::visit([](const auto &value) { return value.id_; }, component);
}

/// Label drawn in the left column, or nullptr for full-width components.
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

/// Width the content of a component needs (without its label).
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

void DrawBoxText(WrappedRectangle box, const std::string &text,
                 WrappedColor color, bool keep_end = false) {
    const std::string kShown =
        Fit(text, box.width_ - 2.0F * kControlPadding, keep_end);
    utils::DrawTextWrapped(kShown.c_str(), box.x_ + kControlPadding,
                           box.y_ + 2.0F, kFontSize, color);
}

}  // namespace

namespace editor::ui {

ComponentInput ComponentInput::Read(utils::WrappedVector2 cursor) {
    ComponentInput input;
    input.cursor_ = cursor;
    input.left_pressed_ = utils::IsLeftClicked();
    input.left_down_ = utils::IsLeftDown();
    for (int character = utils::GetCharPressedWrapped(); character > 0;
         character = utils::GetCharPressedWrapped()) {
        AppendCodePoint(input.typed_, character);
    }
    input.backspace_ =
        utils::IsKeyPressedWrapped(utils::WrappedKey::kBackspace) ||
        utils::IsKeyPressedRepeatWrapped(utils::WrappedKey::kBackspace);
    input.enter_ = utils::IsKeyPressedWrapped(utils::WrappedKey::kEnter) ||
                   utils::IsKeyPressedWrapped(utils::WrappedKey::kKpEnter);
    input.escape_ = utils::IsKeyPressedWrapped(utils::WrappedKey::kEscape);
    return input;
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
    // The edited node changed without a click on the fields: drop the edit.
    if (edit_ && edit_->node_id_ != node_id) edit_.reset();
    if (open_list_ && open_list_->node_id_ != node_id) open_list_.reset();

    if (open_list_ && HandleOpenList(components, bounds, input)) {
        return true;
    }

    if (edit_) {
        const auto kIndex = IndexOfId(components, edit_->component_id_);
        if (!kIndex) {
            edit_.reset();
        } else {
            bool changed = false;
            if (!input.typed_.empty()) {
                edit_->text_ += input.typed_;
                changed = true;
            }
            if (input.backspace_ && !edit_->text_.empty()) {
                PopCodePoint(edit_->text_);
                changed = true;
            }
            if (changed) {
                UpdateEditError(components[*kIndex]);
            }
            if (input.escape_) {
                FinishEdit(components, false);
            } else if (input.enter_ && edit_->error_.empty()) {
                FinishEdit(components, true);
            }
        }
    }

    if (!input.left_pressed_) {
        return false;
    }

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
                    if (value.enabled_ && !value.read_only_ && !edit_) {
                        StartEdit(node_id, component);
                    }
                } else if constexpr (std::is_same_v<T, NumberField>) {
                    if (value.enabled_ && !edit_) {
                        StartEdit(node_id, component);
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

void ComponentAdapter::DrawRow(const ComponentList &components,
                               WrappedRectangle bounds, std::size_t index,
                               core::NodeId node_id, bool active) const {
    const Component &component = components[index];
    const WrappedRectangle kRow = RowBounds(bounds, index);
    const WrappedRectangle kControl = ControlBounds(components, bounds, index);
    const bool kEditing = edit_ && edit_->node_id_ == node_id &&
                          edit_->component_id_ == IdOf(component);
    const bool kListOpen = open_list_ && open_list_->node_id_ == node_id &&
                           open_list_->component_id_ == IdOf(component);

    if (const std::string *label = LabelOf(component)) {
        utils::DrawTextWrapped(
            Fit(*label, kControl.x_ - kRow.x_, false).c_str(), kRow.x_,
            kRow.y_ + 4.0F, kFontSize, kMutedText);
    }

    std::visit(
        [&](const auto &value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, Label>) {
                utils::DrawTextWrapped(
                    Fit(value.text_, kRow.width_, false).c_str(), kRow.x_,
                    kRow.y_ + 4.0F, kFontSize, kText);
            } else if constexpr (std::is_same_v<T, InfoBubble>) {
                utils::DrawTextWrapped(value.text_.c_str(), kRow.x_,
                                       kRow.y_ + 4.0F, kFontSize, kWarningText);
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
                    DrawBoxText(kControl, value.label_, kValueColor);
                } else if constexpr (std::is_same_v<T, TextField>) {
                    if (kEditing) {
                        DrawBoxText(kControl, edit_->text_ + "|", kText, true);
                    } else {
                        DrawBoxText(kControl, value.value_, kValueColor);
                    }
                } else if constexpr (std::is_same_v<T, NumberField>) {
                    if (kEditing) {
                        DrawBoxText(kControl, edit_->text_ + "|", kText, true);
                    } else {
                        DrawBoxText(
                            kControl,
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
                    DrawBoxText(kControl,
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
                    utils::DrawTextWrapped(value.value_ ? "vrai" : "faux",
                                           kSquare.x_ + kSize + 6.0F,
                                           kControl.y_ + 2.0F, kFontSize,
                                           kValueColor);
                } else if constexpr (std::is_same_v<T, Select>) {
                    const std::string kSelected =
                        value.selected_index_ < value.options_.size()
                            ? value.options_[value.selected_index_]
                            : std::string{"<invalide>"};
                    DrawBoxText({kControl.x_, kControl.y_,
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
            utils::DrawTextWrapped(edit_->error_.c_str(),
                                   kBubble.x_ + kControlPadding,
                                   kBubble.y_ + 3.0F, kFontSize, kText);
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
            DrawBoxText(kOption, select->options_[option], kText);
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
                            WrappedRectangle bounds, bool active) const {
    for (std::size_t index = 0; index < components.size(); ++index) {
        DrawRow(components, bounds, index, node_id, active);
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
