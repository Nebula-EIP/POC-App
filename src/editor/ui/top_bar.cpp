/**
 * @file top_bar.cpp
 * @brief Implementation of the TopBar class and of the Menu builder.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 29-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 29-09-2026
 */

#include "top_bar.hpp"

#include <algorithm>
#include <utility>

namespace editor::ui {

namespace {
constexpr float kDropdownPadding = 4.0F;
constexpr float kShadowOffset = 3.0F;
constexpr utils::WrappedColor kShadow = {0, 0, 0, 70};

bool Contains(utils::WrappedRectangle rectangle, utils::WrappedVector2 point) {
    return point.x_ >= rectangle.x_ &&
           point.x_ < rectangle.x_ + rectangle.width_ &&
           point.y_ >= rectangle.y_ &&
           point.y_ < rectangle.y_ + rectangle.height_;
}
}  // namespace

// ---------------------------------------------------------------------------
// Menu

Menu::Menu(std::string title) : title_(std::move(title)) {}

Menu &Menu::AddAction(std::string label, Command command) {
    items_.push_back({std::move(label), std::move(command), false});
    return *this;
}

Menu &Menu::AddSeparator() {
    items_.push_back({"", {}, true});
    return *this;
}

// ---------------------------------------------------------------------------
// TopBar: layout

TopBar::TopBar(TopBarStyle style, TextMeasurer measurer)
    : style_(style), measurer_(std::move(measurer)) {
    if (!measurer_) {
        measurer_ = [](const std::string &text, int font_size) {
            return utils::MeasureTextWrapped(text.c_str(), font_size);
        };
    }
}

Menu &TopBar::AddMenu(std::string title) {
    Close();
    menus_.emplace_back(std::move(title));
    return menus_.back();
}

float TopBar::TitleWidth(const Menu &menu) const {
    return measurer_(menu.Title(), style_.font_size_) +
           2.0F * style_.title_padding_;
}

float TopBar::ItemHeight(const MenuItem &item) const {
    return item.separator_ ? style_.separator_height_ : style_.item_height_;
}

utils::WrappedRectangle TopBar::TitleRect(std::size_t menu) const {
    float x = 0.0F;
    for (std::size_t index = 0; index < menu && index < menus_.size();
         ++index) {
        x += TitleWidth(menus_[index]);
    }
    const float kWidth = menu < menus_.size() ? TitleWidth(menus_[menu]) : 0.0F;
    return {x, 0.0F, kWidth, style_.height_};
}

utils::WrappedRectangle TopBar::DropdownRect() const {
    if (!open_menu_.has_value() || *open_menu_ >= menus_.size()) {
        return {0.0F, 0.0F, 0.0F, 0.0F};
    }
    const Menu &menu = menus_[*open_menu_];

    float width = style_.min_dropdown_width_;
    float height = 2.0F * kDropdownPadding;
    for (const MenuItem &item : menu.Items()) {
        width = std::max(width, measurer_(item.label_, style_.font_size_) +
                                    2.0F * style_.item_padding_);
        height += ItemHeight(item);
    }

    // Shifted left when it would leave the window.
    float x = TitleRect(*open_menu_).x_;
    if (screen_width_ > 0.0F && x + width > screen_width_) {
        x = std::max(screen_width_ - width, 0.0F);
    }
    return {x, style_.height_, width, height};
}

utils::WrappedRectangle TopBar::ItemRect(std::size_t item) const {
    if (!open_menu_.has_value() || *open_menu_ >= menus_.size() ||
        item >= menus_[*open_menu_].Items().size()) {
        return {0.0F, 0.0F, 0.0F, 0.0F};
    }
    const std::vector<MenuItem> &items = menus_[*open_menu_].Items();
    const utils::WrappedRectangle kDropdown = DropdownRect();
    float y = kDropdown.y_ + kDropdownPadding;
    for (std::size_t index = 0; index < item; ++index) {
        y += ItemHeight(items[index]);
    }
    return {kDropdown.x_, y, kDropdown.width_, ItemHeight(items[item])};
}

TopBar::Hit TopBar::HitTest(utils::WrappedVector2 point) const {
    Hit hit;
    hit.on_bar_ = point.y_ >= 0.0F && point.y_ < style_.height_ &&
                  point.x_ >= 0.0F &&
                  (screen_width_ <= 0.0F || point.x_ < screen_width_);
    if (hit.on_bar_) {
        for (std::size_t menu = 0; menu < menus_.size(); ++menu) {
            if (Contains(TitleRect(menu), point)) {
                hit.title_ = menu;
                break;
            }
        }
        return hit;
    }

    hit.in_dropdown_ = IsOpen() && Contains(DropdownRect(), point);
    if (hit.in_dropdown_) {
        for (std::size_t item = 0; item < menus_[*open_menu_].Items().size();
             ++item) {
            if (Contains(ItemRect(item), point)) {
                hit.item_ = item;
                break;
            }
        }
    }
    return hit;
}

// ---------------------------------------------------------------------------
// TopBar: input

TopBarInput TopBar::ReadInput() {
    TopBarInput input;
    input.cursor_ = utils::GetCursorPositionWrapped();
    input.left_pressed_ = utils::IsLeftClicked();
    input.other_pressed_ = utils::IsRightClicked() || utils::IsMiddleClicked();
    input.any_down_ =
        utils::IsLeftDown() || utils::IsRightDown() || utils::IsMiddleDown();
    input.escape_pressed_ =
        utils::IsKeyPressedWrapped(utils::WrappedKey::kEscape);
    input.screen_width_ = utils::GetScreenWidthWrapped();
    return input;
}

bool TopBar::Update(const TopBarInput &input) {
    screen_width_ = input.screen_width_;
    const bool kWasOpen = IsOpen();

    if (input.escape_pressed_) {
        Close();
    }

    const Hit kHit = HitTest(input.cursor_);
    const bool kOverBar = kHit.on_bar_ || kHit.in_dropdown_;

    // A press decides who owns the mouse until every button is released: the
    // bar when the press lands on it or closes an open menu, the canvas
    // otherwise. This keeps a canvas drag alive when it crosses the bar.
    if (input.left_pressed_ || input.other_pressed_) {
        pointer_owned_ = kOverBar || IsOpen();
    }
    const bool kCanvasOwnsPointer = input.any_down_ && !pointer_owned_;

    hovered_title_ = kCanvasOwnsPointer ? std::nullopt : kHit.title_;
    hovered_item_ = kCanvasOwnsPointer ? std::nullopt : kHit.item_;

    Command command;
    if (input.left_pressed_ && !kCanvasOwnsPointer) {
        command = HandleClick(kHit);
    } else if (input.other_pressed_ && !kOverBar) {
        Close();
    } else if (IsOpen() && kHit.title_.has_value()) {
        // Moving over another title while a menu is open switches to it.
        open_menu_ = kHit.title_;
    }

    const bool kCaptured = pointer_owned_ || kWasOpen || IsOpen() ||
                           (kOverBar && !input.any_down_);
    if (!input.any_down_) {
        pointer_owned_ = false;
    }

    if (command) {
        command();
    }
    return kCaptured;
}

Command TopBar::HandleClick(const Hit &hit) {
    if (hit.title_.has_value()) {
        if (open_menu_ == hit.title_) {
            Close();
        } else {
            open_menu_ = hit.title_;
        }
        return {};
    }
    if (!hit.in_dropdown_) {
        // Empty part of the bar, or anywhere outside the menu.
        Close();
        return {};
    }
    if (!hit.item_.has_value()) {
        return {};
    }

    const MenuItem &item = menus_[*open_menu_].Items()[*hit.item_];
    if (!item.IsEnabled()) {
        return {};  // Greyed out items keep the menu open.
    }
    Command command = item.command_;
    Close();
    return command;
}

// ---------------------------------------------------------------------------
// TopBar: drawing

void TopBar::Draw() const {
    const float kWidth =
        screen_width_ > 0.0F ? screen_width_ : utils::GetScreenWidthWrapped();
    const float kFont = static_cast<float>(style_.font_size_);

    utils::DrawRectangleWrapped(0.0F, 0.0F, kWidth, style_.height_,
                                style_.bar_);
    utils::DrawRectangleWrapped(0.0F, style_.height_ - 1.0F, kWidth, 1.0F,
                                style_.border_);

    for (std::size_t menu = 0; menu < menus_.size(); ++menu) {
        const utils::WrappedRectangle kRect = TitleRect(menu);
        if (open_menu_ == menu || hovered_title_ == menu) {
            utils::DrawRectangleWrapped(kRect.x_, kRect.y_, kRect.width_,
                                        kRect.height_ - 1.0F, style_.hover_);
        }
        utils::DrawTextWrapped(
            menus_[menu].Title().c_str(), kRect.x_ + style_.title_padding_,
            (style_.height_ - kFont) / 2.0F, style_.font_size_, style_.text_);
    }

    if (!open_menu_.has_value() || *open_menu_ >= menus_.size()) {
        return;
    }

    const utils::WrappedRectangle kDropdown = DropdownRect();
    utils::DrawRectangleWrapped(kDropdown.x_ + kShadowOffset,
                                kDropdown.y_ + kShadowOffset, kDropdown.width_,
                                kDropdown.height_, kShadow);
    utils::DrawRectangleWrapped(kDropdown.x_, kDropdown.y_, kDropdown.width_,
                                kDropdown.height_, style_.dropdown_);
    utils::DrawRectangleLinesWrapped(kDropdown.x_, kDropdown.y_,
                                     kDropdown.width_, kDropdown.height_,
                                     style_.border_);

    const std::vector<MenuItem> &items = menus_[*open_menu_].Items();
    for (std::size_t index = 0; index < items.size(); ++index) {
        const MenuItem &item = items[index];
        const utils::WrappedRectangle kItem = ItemRect(index);

        if (item.separator_) {
            utils::DrawRectangleWrapped(kItem.x_ + style_.item_padding_ / 2.0F,
                                        kItem.y_ + kItem.height_ / 2.0F,
                                        kItem.width_ - style_.item_padding_,
                                        1.0F, style_.border_);
            continue;
        }

        const bool kEnabled = item.IsEnabled();
        if (kEnabled && hovered_item_ == index) {
            utils::DrawRectangleWrapped(kItem.x_ + 1.0F, kItem.y_,
                                        kItem.width_ - 2.0F, kItem.height_,
                                        style_.hover_);
        }
        utils::DrawTextWrapped(
            item.label_.c_str(), kItem.x_ + style_.item_padding_,
            kItem.y_ + (kItem.height_ - kFont) / 2.0F, style_.font_size_,
            kEnabled ? style_.text_ : style_.disabled_text_);
    }
}

}  // namespace editor::ui
