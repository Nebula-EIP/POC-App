/**
 * @file palette.hpp
 * @brief The project color palette: every color the editor draws with, by
 * name. The values live in palette.yaml, read at start-up.
 *
 * Referencing a color: palette::Get().<section>_.<name>_, e.g.
 * palette::Get().canvas_.background_. Pin types and node accents are looked
 * up by name: palette::TypeColor("int"), palette::NodeAccent("Variable",
 * "Data").
 *
 * Adding a color: one line in palette.yaml and one member (plus its entry in
 * Palette::Fields(), palette.cpp) here. A color only added to the type_names,
 * accent_nodes or accent_categories tables of palette.yaml needs no code.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 05-11-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 17-11-2026
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "utils/raylib_wrapper.hpp"

namespace palette {

/// The palette file read at start-up, from the working directory.
inline constexpr const char *kDefaultFile = "palette.yaml";

/// Name of the type of the execution flow pins, drawn as triangles.
inline constexpr std::string_view kFlowTypeName = "void";

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

/// Colors by name (type names, node names, categories).
using ColorTable = std::map<std::string, utils::WrappedColor, std::less<>>;

/**
 * @brief Every color of the editor. One member per entry of palette.yaml,
 * one struct per section.
 */
struct Palette {
    /// The Chasm palette of the design: the colors the rest is built from.
    struct Chasm {
        utils::WrappedColor silver_;
        utils::WrappedColor slate_;
        utils::WrappedColor charcoal_;
        utils::WrappedColor abyss_;
        utils::WrappedColor green_;
        utils::WrappedColor red_;
        utils::WrappedColor violet_;
        utils::WrappedColor orange_;
        utils::WrappedColor blue_;
    };

    /// Colors shared by several parts of the editor.
    struct Base {
        utils::WrappedColor white_;
        utils::WrappedColor shadow_;
        utils::WrappedColor accent_;
        utils::WrappedColor error_;
        utils::WrappedColor warning_;
    };

    /// The node canvas: background, grid, rectangle selection.
    struct Canvas {
        utils::WrappedColor background_;
        utils::WrappedColor grid_;
        utils::WrappedColor loading_text_;
        utils::WrappedColor selection_fill_;
        utils::WrappedColor selection_border_;
    };

    /// The menu bar at the top of the window and its drop-down menus.
    struct TopBar {
        utils::WrappedColor background_;
        utils::WrappedColor text_;
        utils::WrappedColor text_hover_;
        utils::WrappedColor item_text_;
        utils::WrappedColor disabled_text_;
        utils::WrappedColor shadow_;
        utils::WrappedColor hover_;
        utils::WrappedColor dropdown_;
        utils::WrappedColor border_;
    };

    /// The node cards.
    struct Node {
        utils::WrappedColor header_;
        utils::WrappedColor body_;
        utils::WrappedColor title_;
        utils::WrappedColor pin_label_;
        utils::WrappedColor selected_outline_;
        utils::WrappedColor shadow_;
        utils::WrappedColor error_pin_;
    };

    /// The node picker ("Add Node"), drawn like a node card.
    struct Popup {
        utils::WrappedColor header_;
        utils::WrappedColor background_;
        utils::WrappedColor search_background_;
        utils::WrappedColor search_border_;
        utils::WrappedColor placeholder_;
        utils::WrappedColor text_;
        utils::WrappedColor folder_text_;
        utils::WrappedColor selected_;
        utils::WrappedColor selected_text_;
        utils::WrappedColor empty_text_;
        utils::WrappedColor scrollbar_;
        utils::WrappedColor shadow_;
    };

    /// The values and settings drawn inside the nodes.
    struct Field {
        utils::WrappedColor label_;
        utils::WrappedColor value_;
        utils::WrappedColor disabled_text_;
        utils::WrappedColor warning_text_;
        utils::WrappedColor idle_;
        utils::WrappedColor active_;
        utils::WrappedColor editing_;
        /// Underline of the text fields, outline and mark of the checkboxes.
        utils::WrappedColor stroke_;
        utils::WrappedColor accent_;
        utils::WrappedColor error_;
        utils::WrappedColor error_bubble_;
        utils::WrappedColor error_text_;
        utils::WrappedColor list_background_;
        utils::WrappedColor list_selected_;
        utils::WrappedColor list_text_;
        utils::WrappedColor separator_;
    };

    /// The links between pins (their color is the one of their type).
    struct Link {
        utils::WrappedColor selected_;
        utils::WrappedColor ghost_;
    };

    /// Warnings stacked at the bottom of the window.
    struct Notification {
        utils::WrappedColor background_;
        utils::WrappedColor border_;
        utils::WrappedColor text_;
    };

    /// Pins and links of the types not listed in type_names_.
    struct Type {
        /// Types defined by the user (structures...).
        utils::WrappedColor custom_;
        /// Every other type (char...).
        utils::WrappedColor other_;
    };

    /// Line at the top of the nodes.
    struct Accent {
        /// Fully transparent: no line.
        utils::WrappedColor none_;
        /// Entry points: functions, their inputs and outputs.
        utils::WrappedColor entry_;
        /// Branches (if...) and loops.
        utils::WrappedColor branch_;
        /// Nodes that set a variable.
        utils::WrappedColor variable_;
    };

    Chasm chasm_{};
    Base base_{};
    Canvas canvas_{};
    TopBar top_bar_{};
    Node node_{};
    Popup popup_{};
    Field field_{};
    Link link_{};
    Notification notification_{};
    Type type_{};
    /// Color of each type, by the name the modules give it.
    ColorTable type_names_;
    Accent accent_{};
    /// Accent of single nodes, by node name. Wins over the category.
    ColorTable accent_nodes_;
    /// Accent of each category. Sub-folders ("Flow/Loops") use their root's.
    ColorTable accent_categories_;

    /**
     * @brief The palette compiled into the editor (the palette.yaml of the
     * build).
     */
    [[nodiscard]] static Palette Default();

    /**
     * @brief Reads a palette from YAML text.
     *
     * Colors the text leaves out keep their Default() value. A color that
     * cannot be read (bad hex code, unknown or circular reference...) also
     * keeps it, with a message in @p warnings.
     *
     * @param yaml The contents of a palette file.
     * @param warnings Receives one message per ignored entry.
     *
     * @throws std::runtime_error If the text is not valid YAML.
     */
    [[nodiscard]] static Palette FromYaml(std::string_view yaml,
                                          std::vector<std::string> &warnings);

    /**
     * @brief Reads a palette file, see FromYaml().
     *
     * @param file The palette file, e.g. kDefaultFile.
     * @param warnings Receives one message per ignored entry, or one saying
     * the file is missing (the palette is then Default()).
     *
     * @throws std::runtime_error If the file cannot be read or is not valid
     * YAML.
     */
    [[nodiscard]] static Palette Load(const std::filesystem::path &file,
                                      std::vector<std::string> &warnings);

    /**
     * @brief Color of the pins and links of a type.
     *
     * @param type_name The name of the type, e.g. "int".
     * @param is_custom Whether the user defined the type (structure...).
     *
     * @return type_.custom_ for a custom type, else its color in type_names_,
     * else type_.other_.
     */
    [[nodiscard]] utils::WrappedColor TypeColor(std::string_view type_name,
                                                bool is_custom = false) const;

    /**
     * @brief Color of the line at the top of a node.
     *
     * @param node_name The name of the node, e.g. "Variable".
     * @param category The category of the node, e.g. "Flow/Loops".
     *
     * @return The node's color in accent_nodes_, else the color of its root
     * folder in accent_categories_, else accent_.none_.
     */
    [[nodiscard]] utils::WrappedColor NodeAccent(
        std::string_view node_name, std::string_view category) const;

    /**
     * @brief Every fixed color of the palette with its "section.name" in
     * palette.yaml (the tables are not included).
     */
    [[nodiscard]] std::vector<
        std::pair<std::string_view, utils::WrappedColor *>>
    Fields();
};

/**
 * @brief The palette the editor draws with: Palette::Default() until Set()
 * is called.
 */
[[nodiscard]] const Palette &Get();

/// Replaces the palette the editor draws with (e.g. by Palette::Load()).
void Set(Palette palette);

/// Get().TypeColor(), see Palette::TypeColor().
[[nodiscard]] utils::WrappedColor TypeColor(std::string_view type_name,
                                            bool is_custom = false);

/// Get().NodeAccent(), see Palette::NodeAccent().
[[nodiscard]] utils::WrappedColor NodeAccent(std::string_view node_name,
                                             std::string_view category);

}  // namespace palette
