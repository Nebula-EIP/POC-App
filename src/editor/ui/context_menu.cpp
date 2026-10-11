/**
 * @file context_menu.cpp
 * @brief Implementation of ContextMenu.
 *
 * @author Created by mathys-f
 * @date Created on 06-10-2026
 */

#include "context_menu.hpp"

#include <algorithm>

namespace editor::ui {

bool ContextMenuItem::IsEnabled() const {
    return !separator_ && static_cast<bool>(command_);
}

ContextMenu::ContextMenu(TopBarStyle style, TextMeasurer measurer)
    : style_(style), measurer_(std::move(measurer)) {
    if (!measurer_) {
        measurer_ = [](const std::string &text, int font_size) {
            return utils::MeasureTextWrapped(text.c_str(), font_size);
        };
    }
}

ContextMenu &ContextMenu::AddAction(const std::string &label,
                                    actions::ActionCallback command,
                                    const std::string &shortcut) {
    items_.push_back(
        ContextMenuItem{label, std::move(command), shortcut, false});
    return *this;
}

ContextMenu &ContextMenu::AddSeparator() {
    items_.push_back(ContextMenuItem{"", {}, "", true});
    return *this;
}

void ContextMenu::Clear() {
    items_.clear();
    is_open_ = false;
}

float ContextMenu::ItemHeight(const ContextMenuItem &item) const {
    return item.separator_ ? style_.separator_height_ : style_.item_height_;
}

float ContextMenu::MenuWidth() const {
    float max_width = style_.min_dropdown_width_;
    for (const auto &item : items_) {
        if (!item.separator_) {
            float width = style_.item_padding_ * 2.0F +
                          measurer_(item.label_, style_.font_size_);
            if (!item.shortcut_text_.empty()) {
                width +=
                    20.0F + measurer_(item.shortcut_text_, style_.font_size_);
            }
            max_width = std::max(max_width, width);
        }
    }
    return max_width;
}

utils::WrappedRectangle ContextMenu::Bounds() const {
    float total_height = 0.0F;
    for (const auto &item : items_) {
        total_height += ItemHeight(item);
    }
    return utils::WrappedRectangle{position_.x_, position_.y_, MenuWidth(),
                                   total_height};
}

void ContextMenu::OpenAt(utils::WrappedVector2 position, float screen_width,
                         float screen_height) {
    is_open_ = true;
    position_ = position;

    // Clamp to screen bounds
    const auto kBounds = Bounds();
    if (position_.x_ + kBounds.width_ > screen_width) {
        position_.x_ = std::max(0.0F, screen_width - kBounds.width_);
    }
    if (position_.y_ + kBounds.height_ > screen_height) {
        position_.y_ = std::max(0.0F, screen_height - kBounds.height_);
    }
}

std::optional<std::size_t> ContextMenu::HitTest(
    utils::WrappedVector2 point) const {
    const auto kBounds = Bounds();
    if (!utils::CheckCollisionPointRecWrapped(point, kBounds)) {
        return std::nullopt;
    }

    float current_y = kBounds.y_;
    std::size_t i = 0;
    for (const auto &item : items_) {
        const float kHeight = ItemHeight(item);
        utils::WrappedRectangle item_rect{kBounds.x_, current_y, kBounds.width_,
                                          kHeight};

        if (utils::CheckCollisionPointRecWrapped(point, item_rect)) {
            if (item.IsEnabled()) {
                return i;
            }
            return std::nullopt;
        }
        current_y += kHeight;
        ++i;
    }

    return std::nullopt;
}

bool ContextMenu::Update(const ContextMenuInput &input) {
    if (!is_open_) return false;

    if (input.escape_pressed_) {
        Close();
        return true;
    }

    hovered_item_ = HitTest(input.cursor_);

    if (input.left_pressed_ || input.right_pressed_) {
        if (hovered_item_.has_value()) {
            if (items_[*hovered_item_].command_) {
                items_[*hovered_item_].command_();
            }
            Close();
            return true;
        } else {
            // Clicked outside
            Close();
            // We return false so the application can process the click (e.g.
            // deselecting nodes)
            return false;
        }
    }

    return true;  // We consume hover input while open
}

void ContextMenu::Draw() const {
    if (!is_open_ || items_.empty()) return;

    const auto kBounds = Bounds();

    // Background and border
    utils::DrawRectangleWrapped(kBounds.x_, kBounds.y_, kBounds.width_,
                                kBounds.height_, style_.dropdown_);
    utils::DrawRectangleLinesWrapped(kBounds.x_, kBounds.y_, kBounds.width_,
                                     kBounds.height_, style_.border_);

    float current_y = kBounds.y_;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        const auto &item = items_[i];
        const float kHeight = ItemHeight(item);
        const float kItemCenterY = current_y + kHeight / 2.0F;

        if (item.separator_) {
            utils::DrawLineWrapped(
                utils::WrappedVector2{kBounds.x_, kItemCenterY},
                utils::WrappedVector2{kBounds.x_ + kBounds.width_,
                                      kItemCenterY},
                1.0F, style_.border_);
        } else {
            // Hover background
            if (hovered_item_ == i) {
                utils::DrawRectangleWrapped(kBounds.x_, current_y,
                                            kBounds.width_, kHeight,
                                            style_.hover_);
            }

            // Label
            utils::WrappedColor text_color =
                item.IsEnabled() ? style_.text_ : style_.disabled_text_;
            utils::DrawTextWrapped(
                item.label_.c_str(), kBounds.x_ + style_.item_padding_,
                kItemCenterY - static_cast<float>(style_.font_size_) / 2.0F,
                style_.font_size_, text_color);

            // Shortcut
            if (!item.shortcut_text_.empty()) {
                const float kTextWidth =
                    measurer_(item.shortcut_text_, style_.font_size_);
                utils::DrawTextWrapped(
                    item.shortcut_text_.c_str(),
                    kBounds.x_ + kBounds.width_ - style_.item_padding_ -
                        kTextWidth,
                    kItemCenterY - static_cast<float>(style_.font_size_) / 2.0F,
                    style_.font_size_, style_.disabled_text_);
            }
        }

        current_y += kHeight;
    }
}

}  // namespace editor::ui
