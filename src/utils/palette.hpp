/**
 * @file palette.hpp
 * @brief The project color palette: every color the editor draws with, by
 * name, in one place.
 *
 * Adding a color: one line in the section of the part of the editor it
 * belongs to, from its hex code and an optional opacity:
 * @code
 * inline constexpr utils::WrappedColor kMyColor = Hex(0x23252D);
 * inline constexpr utils::WrappedColor kMyShadow = Hex(0x000000, 0.25F);
 * @endcode
 * A color can reuse another one by name (kText = top_bar::kText).
 *
 * Referencing a color: palette::<section>::k<Name>, e.g.
 * palette::canvas::kBackground.
 *
 * Pin types and node accents are looked up by name (TypeColor("int"),
 * NodeAccent("Variable", "Data")): adding one is one line in its table.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 05-11-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 05-11-2026
 */

#pragma once

#include <cstdint>
#include <string_view>

#include "utils/raylib_wrapper.hpp"

namespace palette {

/**
 * @brief Builds a color from its hex code, as written in a design tool.
 *
 * @param rgb The color as 0xRRGGBB, e.g. 0x23252D.
 * @param opacity The opacity, from 0 (invisible) to 1 (opaque).
 */
[[nodiscard]] constexpr utils::WrappedColor Hex(std::uint32_t rgb,
                                                float opacity = 1.0F) {
    constexpr std::uint32_t kByte = 0xFFU;
    constexpr float kMaxAlpha = 255.0F;
    const float kClamped =
        opacity < 0.0F ? 0.0F : (opacity > 1.0F ? 1.0F : opacity);
    return utils::WrappedColor{
        static_cast<unsigned char>((rgb >> 16U) & kByte),
        static_cast<unsigned char>((rgb >> 8U) & kByte),
        static_cast<unsigned char>(rgb & kByte),
        static_cast<unsigned char>(kClamped * kMaxAlpha + 0.5F),
    };
}

/**
 * @brief The same color with another opacity.
 *
 * @param color The color to change.
 * @param opacity The new opacity, from 0 (invisible) to 1 (opaque).
 */
[[nodiscard]] constexpr utils::WrappedColor WithOpacity(
    utils::WrappedColor color, float opacity) {
    constexpr float kMaxAlpha = 255.0F;
    const float kClamped =
        opacity < 0.0F ? 0.0F : (opacity > 1.0F ? 1.0F : opacity);
    color.a_ = static_cast<unsigned char>(kClamped * kMaxAlpha + 0.5F);
    return color;
}

/**
 * @brief The Chasm palette of the design: the colors the rest of the palette
 * is built from.
 */
namespace chasm {
inline constexpr utils::WrappedColor kSilver = Hex(0xAEAEB4);
inline constexpr utils::WrappedColor kSlate = Hex(0x4E4F56);
inline constexpr utils::WrappedColor kCharcoal = Hex(0x24252C);
inline constexpr utils::WrappedColor kAbyss = Hex(0x16171D);
inline constexpr utils::WrappedColor kGreen = Hex(0x6EA17D);
inline constexpr utils::WrappedColor kRed = Hex(0xA8474A);
inline constexpr utils::WrappedColor kViolet = Hex(0x7E7FCD);
inline constexpr utils::WrappedColor kOrange = Hex(0xD19D82);
inline constexpr utils::WrappedColor kBlue = Hex(0x76A1E1);
}  // namespace chasm

/// Colors shared by several parts of the editor.
namespace base {
inline constexpr utils::WrappedColor kWhite = Hex(0xFFFFFF);
inline constexpr utils::WrappedColor kShadow = Hex(0x000000, 0.25F);
inline constexpr utils::WrappedColor kAccent = chasm::kBlue;
inline constexpr utils::WrappedColor kError = Hex(0xD9575B);
inline constexpr utils::WrappedColor kWarning = Hex(0xE8C15A);
}  // namespace base

/// The node canvas: background, grid, rectangle selection.
namespace canvas {
inline constexpr utils::WrappedColor kBackground = Hex(0x23252D);
inline constexpr utils::WrappedColor kGrid = Hex(0x2E3037);
inline constexpr utils::WrappedColor kLoadingText = Hex(0x909195);
inline constexpr utils::WrappedColor kSelectionFill =
    WithOpacity(base::kAccent, 0.12F);
inline constexpr utils::WrappedColor kSelectionBorder =
    WithOpacity(base::kAccent, 0.8F);
}  // namespace canvas

/// The menu bar at the top of the window and its drop-down menus.
namespace top_bar {
inline constexpr utils::WrappedColor kBackground = Hex(0x22232C);
inline constexpr utils::WrappedColor kText = Hex(0x909195);
inline constexpr utils::WrappedColor kTextHover = Hex(0xF2F2F4);
inline constexpr utils::WrappedColor kItemText = Hex(0xC9CACE);
inline constexpr utils::WrappedColor kDisabledText = Hex(0x6A6B72);
inline constexpr utils::WrappedColor kShadow = base::kShadow;
inline constexpr utils::WrappedColor kHover = Hex(0x30313B);
inline constexpr utils::WrappedColor kDropdown = Hex(0x22232C);
inline constexpr utils::WrappedColor kBorder = Hex(0x33343E);
}  // namespace top_bar

/// The node cards.
namespace node {
inline constexpr utils::WrappedColor kHeader = Hex(0x1F2027);
inline constexpr utils::WrappedColor kBody = chasm::kAbyss;
inline constexpr utils::WrappedColor kTitle = base::kWhite;
inline constexpr utils::WrappedColor kPinLabel = Hex(0xE2E2E4);
inline constexpr utils::WrappedColor kSelectedOutline = Hex(0xD7D8DD);
inline constexpr utils::WrappedColor kShadow = Hex(0x000000, 0.18F);
inline constexpr utils::WrappedColor kErrorPin = base::kError;
}  // namespace node

/// The node picker ("Add Node"), drawn like a node card.
namespace popup {
inline constexpr utils::WrappedColor kHeader = node::kHeader;
inline constexpr utils::WrappedColor kBackground = node::kBody;
inline constexpr utils::WrappedColor kSearchBackground = chasm::kAbyss;
inline constexpr utils::WrappedColor kSearchBorder = chasm::kCharcoal;
inline constexpr utils::WrappedColor kPlaceholder = top_bar::kText;
inline constexpr utils::WrappedColor kText = node::kPinLabel;
inline constexpr utils::WrappedColor kFolderText = chasm::kSilver;
inline constexpr utils::WrappedColor kSelected = chasm::kCharcoal;
inline constexpr utils::WrappedColor kSelectedText = base::kWhite;
inline constexpr utils::WrappedColor kEmptyText = top_bar::kDisabledText;
inline constexpr utils::WrappedColor kScrollbar = chasm::kSlate;
inline constexpr utils::WrappedColor kShadow = base::kShadow;
}  // namespace popup

/// The values and settings drawn inside the nodes.
namespace field {
inline constexpr utils::WrappedColor kLabel = Hex(0xA9AAAF);
inline constexpr utils::WrappedColor kValue = Hex(0xC4C5C7);
inline constexpr utils::WrappedColor kDisabledText = Hex(0x6B6C72);
inline constexpr utils::WrappedColor kWarningText = base::kWarning;
inline constexpr utils::WrappedColor kIdle = node::kHeader;
inline constexpr utils::WrappedColor kActive = Hex(0x262730);
inline constexpr utils::WrappedColor kEditing = Hex(0x2D2E38);
/// Underline of the text fields, outline and mark of the checkboxes.
inline constexpr utils::WrappedColor kStroke = Hex(0xBBBBBB);
inline constexpr utils::WrappedColor kAccent = base::kAccent;
inline constexpr utils::WrappedColor kError = base::kError;
inline constexpr utils::WrappedColor kErrorBubble = Hex(0x6E2A2D, 0.95F);
inline constexpr utils::WrappedColor kErrorText = base::kWhite;
inline constexpr utils::WrappedColor kListBackground = top_bar::kDropdown;
inline constexpr utils::WrappedColor kListSelected = top_bar::kHover;
inline constexpr utils::WrappedColor kListText = top_bar::kTextHover;
inline constexpr utils::WrappedColor kSeparator = Hex(0x34353D);
}  // namespace field

/// The links between pins (their color is the one of their type).
namespace link {
inline constexpr utils::WrappedColor kSelected = base::kWhite;
inline constexpr utils::WrappedColor kGhost = WithOpacity(chasm::kSilver, 0.5F);
}  // namespace link

/// Warnings stacked at the bottom of the window.
namespace notification {
inline constexpr utils::WrappedColor kBackground = Hex(0x3A2226, 0.95F);
inline constexpr utils::WrappedColor kBorder = base::kError;
inline constexpr utils::WrappedColor kText = base::kWhite;
}  // namespace notification

/// Colors of the pin types, also used by the links of that type.
namespace type {
inline constexpr utils::WrappedColor kFlow = chasm::kSilver;
inline constexpr utils::WrappedColor kInt = chasm::kBlue;
inline constexpr utils::WrappedColor kFloat = chasm::kViolet;
inline constexpr utils::WrappedColor kString = chasm::kGreen;
inline constexpr utils::WrappedColor kBool = chasm::kRed;
/// Types defined by the user (structures...).
inline constexpr utils::WrappedColor kCustom = chasm::kOrange;
/// Every other type (char...).
inline constexpr utils::WrappedColor kOther = chasm::kSlate;

/// Name of the type of the execution flow pins, drawn as triangles.
inline constexpr std::string_view kFlowTypeName = "void";

struct Entry {
    std::string_view name_;
    utils::WrappedColor color_;
};

/// Color of each type, by the name the modules give it.
inline constexpr Entry kColors[] = {
    {kFlowTypeName, kFlow}, {"int", kInt},   {"float", kFloat},
    {"string", kString},    {"bool", kBool},
};
}  // namespace type

/**
 * @brief Color of the pins and links of a type.
 *
 * @param type_name The name of the type, e.g. "int".
 * @param is_custom Whether the user defined the type (structure...).
 *
 * @return type::kCustom for a custom type, else its color in type::kColors,
 * else type::kOther.
 */
[[nodiscard]] constexpr utils::WrappedColor TypeColor(
    std::string_view type_name, bool is_custom = false) {
    if (is_custom) {
        return type::kCustom;
    }
    for (const type::Entry &entry : type::kColors) {
        if (entry.name_ == type_name) {
            return entry.color_;
        }
    }
    return type::kOther;
}

/// Line at the top of the nodes, by node and by category.
namespace accent {
inline constexpr utils::WrappedColor kNone = Hex(0x000000, 0.0F);
/// Entry points: functions, their inputs and outputs.
inline constexpr utils::WrappedColor kEntry = chasm::kRed;
/// Branches (if...) and loops.
inline constexpr utils::WrappedColor kBranch = chasm::kSilver;
/// Nodes that set a variable.
inline constexpr utils::WrappedColor kVariable = chasm::kBlue;

struct Entry {
    std::string_view name_;
    utils::WrappedColor color_;
};

/// Accent of single nodes, by node name. Wins over the category.
inline constexpr Entry kNodes[] = {
    {"Variable", kVariable},
};

/// Accent of each category. Sub-folders ("Flow/Loops") use their root's.
inline constexpr Entry kCategories[] = {
    {"Functions", kEntry},
    {"Flow", kBranch},
};
}  // namespace accent

/**
 * @brief Color of the line at the top of a node.
 *
 * @param node_name The name of the node, e.g. "Variable".
 * @param category The category of the node, e.g. "Flow/Loops".
 *
 * @return The node's color in accent::kNodes, else the color of its root
 * folder in accent::kCategories, else accent::kNone (fully transparent: no
 * line).
 */
[[nodiscard]] constexpr utils::WrappedColor NodeAccent(
    std::string_view node_name, std::string_view category) {
    for (const accent::Entry &entry : accent::kNodes) {
        if (entry.name_ == node_name) {
            return entry.color_;
        }
    }
    const std::string_view kRoot = category.substr(0, category.find('/'));
    for (const accent::Entry &entry : accent::kCategories) {
        if (entry.name_ == kRoot) {
            return entry.color_;
        }
    }
    return accent::kNone;
}

}  // namespace palette
