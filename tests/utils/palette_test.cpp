/**
 * @file palette_test.cpp
 * @brief Unit tests for the project color palette (utils/palette.hpp,
 * palette.yaml).
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 05-11-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 17-11-2026
 */

#include "utils/palette.hpp"

#include <gtest/gtest.h>

#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void ExpectColor(utils::WrappedColor color, unsigned char red,
                 unsigned char green, unsigned char blue, unsigned char alpha) {
    EXPECT_EQ(color.r_, red);
    EXPECT_EQ(color.g_, green);
    EXPECT_EQ(color.b_, blue);
    EXPECT_EQ(color.a_, alpha);
}

bool SameColor(utils::WrappedColor left, utils::WrappedColor right) {
    return left.r_ == right.r_ && left.g_ == right.g_ && left.b_ == right.b_ &&
           left.a_ == right.a_;
}

/// Reads @p yaml and expects exactly @p expected_warnings warnings.
palette::Palette Read(const std::string &yaml,
                      std::size_t expected_warnings = 0) {
    std::vector<std::string> warnings;
    palette::Palette result = palette::Palette::FromYaml(yaml, warnings);
    EXPECT_EQ(warnings.size(), expected_warnings);
    for (const std::string &warning : warnings) {
        std::cout << "  warning: " << warning << '\n';
    }
    return result;
}

}  // namespace

TEST(Palette, HexSplitsTheChannelsAndIsOpaqueByDefault) {
    ExpectColor(palette::Hex(0x23252D), 0x23, 0x25, 0x2D, 255);
}

TEST(Palette, HexTakesAnOpacityBetweenZeroAndOne) {
    ExpectColor(palette::Hex(0x000000, 0.25F), 0, 0, 0, 64);
    ExpectColor(palette::Hex(0xFFFFFF, 0.0F), 255, 255, 255, 0);
    // Out of range opacities are clamped.
    EXPECT_EQ(palette::Hex(0x000000, 2.0F).a_, 255);
    EXPECT_EQ(palette::Hex(0x000000, -1.0F).a_, 0);
}

TEST(Palette, WithOpacityKeepsTheColor) {
    ExpectColor(palette::WithOpacity(palette::Hex(0x76A0E0), 0.5F), 0x76, 0xA0,
                0xE0, 128);
}

TEST(Palette, ThePaletteFileIsValidAndComplete) {
    // Every color of the code is written in palette.yaml, without mistake.
    std::vector<std::string> warnings;
    const palette::Palette kLoaded =
        palette::Palette::Load(NEBULA_PALETTE_FILE, warnings);
    EXPECT_TRUE(warnings.empty());
    palette::Palette loaded = kLoaded;
    palette::Palette fallback = palette::Palette::Default();
    const auto kLoadedFields = loaded.Fields();
    const auto kDefaultFields = fallback.Fields();
    ASSERT_EQ(kLoadedFields.size(), kDefaultFields.size());
    for (std::size_t index = 0; index < kLoadedFields.size(); ++index) {
        EXPECT_TRUE(SameColor(*kLoadedFields[index].second,
                              *kDefaultFields[index].second))
            << kLoadedFields[index].first;
    }
}

TEST(Palette, FieldNamesAreUnique) {
    palette::Palette colors;
    std::set<std::string> names;
    for (const auto &[name, color] : colors.Fields()) {
        EXPECT_TRUE(names.emplace(name).second) << name;
    }
}

TEST(Palette, ProjectColorsMatchTheDesign) {
    const palette::Palette kColors = palette::Palette::Default();
    ExpectColor(kColors.canvas_.background_, 0x23, 0x25, 0x2D, 255);
    ExpectColor(kColors.canvas_.grid_, 0x2E, 0x30, 0x37, 255);
    ExpectColor(kColors.top_bar_.background_, 0x22, 0x23, 0x2C, 255);
    ExpectColor(kColors.top_bar_.text_, 0x90, 0x91, 0x95, 255);
    ExpectColor(kColors.top_bar_.shadow_, 0, 0, 0, 64);
    ExpectColor(kColors.canvas_.selection_fill_, 0x76, 0xA1, 0xE1, 31);
}

TEST(Palette, ChasmColorsMatchTheDesign) {
    const palette::Palette::Chasm kChasm = palette::Palette::Default().chasm_;
    ExpectColor(kChasm.silver_, 0xAE, 0xAE, 0xB4, 255);
    ExpectColor(kChasm.slate_, 0x4E, 0x4F, 0x56, 255);
    ExpectColor(kChasm.charcoal_, 0x24, 0x25, 0x2C, 255);
    ExpectColor(kChasm.abyss_, 0x16, 0x17, 0x1D, 255);
    ExpectColor(kChasm.green_, 0x6E, 0xA1, 0x7D, 255);
    ExpectColor(kChasm.red_, 0xA8, 0x47, 0x4A, 255);
    ExpectColor(kChasm.violet_, 0x7E, 0x7F, 0xCD, 255);
    ExpectColor(kChasm.orange_, 0xD1, 0x9D, 0x82, 255);
    ExpectColor(kChasm.blue_, 0x76, 0xA1, 0xE1, 255);
}

TEST(Palette, TypeColorLooksTypesUpByName) {
    const palette::Palette::Chasm kChasm = palette::Palette::Default().chasm_;
    EXPECT_TRUE(SameColor(palette::TypeColor("int"), kChasm.blue_));
    EXPECT_TRUE(SameColor(palette::TypeColor("float"), kChasm.violet_));
    EXPECT_TRUE(SameColor(palette::TypeColor("string"), kChasm.green_));
    EXPECT_TRUE(SameColor(palette::TypeColor("bool"), kChasm.red_));
    EXPECT_TRUE(
        SameColor(palette::TypeColor(palette::kFlowTypeName), kChasm.silver_));
}

TEST(Palette, OtherTypesAreGreyAndCustomTypesOrange) {
    const palette::Palette::Chasm kChasm = palette::Palette::Default().chasm_;
    EXPECT_TRUE(SameColor(palette::TypeColor("char"), kChasm.slate_));
    EXPECT_TRUE(
        SameColor(palette::TypeColor("matrix"), palette::Get().type_.other_));
    EXPECT_TRUE(SameColor(palette::TypeColor("Player", true), kChasm.orange_));
}

TEST(Palette, NodeAccentFollowsTheKindOfNode) {
    // Entry points in red, branches and loops in white, variables in blue.
    const palette::Palette::Chasm kChasm = palette::Palette::Default().chasm_;
    EXPECT_TRUE(
        SameColor(palette::NodeAccent("Function", "Functions"), kChasm.red_));
    EXPECT_TRUE(SameColor(palette::NodeAccent("FunctionOutput", "Functions"),
                          kChasm.red_));
    EXPECT_TRUE(
        SameColor(palette::NodeAccent("Condition", "Flow"), kChasm.silver_));
    EXPECT_TRUE(
        SameColor(palette::NodeAccent("For", "Flow/Loops"), kChasm.silver_));
    EXPECT_TRUE(
        SameColor(palette::NodeAccent("Variable", "Data"), kChasm.blue_));
}

TEST(Palette, NodesWithoutAccentAreTransparent) {
    EXPECT_EQ(palette::NodeAccent("Literal", "Data").a_, 0);
    EXPECT_EQ(palette::NodeAccent("Operator", "Math").a_, 0);
    EXPECT_EQ(palette::NodeAccent("", "").a_, 0);
    // A folder only starting like a known one is not that one.
    EXPECT_EQ(palette::NodeAccent("Rose", "Flowers").a_, 0);
}

TEST(PaletteYaml, AnEmptyFileGivesTheDefaultPalette) {
    const palette::Palette kColors = Read("");
    EXPECT_TRUE(SameColor(kColors.canvas_.background_,
                          palette::Palette::Default().canvas_.background_));
}

TEST(PaletteYaml, AColorOverridesItsDefault) {
    const palette::Palette kColors = Read("canvas:\n  grid: \"#102030\"\n");
    ExpectColor(kColors.canvas_.grid_, 0x10, 0x20, 0x30, 255);
    // The others keep theirs.
    ExpectColor(kColors.canvas_.background_, 0x23, 0x25, 0x2D, 255);
}

TEST(PaletteYaml, ReferencesFollowTheColorTheyName) {
    // base.accent, canvas.selection_fill and the int pins use chasm.blue.
    const palette::Palette kColors = Read("chasm:\n  blue: \"#0000FF\"\n");
    ExpectColor(kColors.base_.accent_, 0, 0, 0xFF, 255);
    ExpectColor(kColors.canvas_.selection_fill_, 0, 0, 0xFF, 31);
    ExpectColor(kColors.TypeColor("int"), 0, 0, 0xFF, 255);
    ExpectColor(kColors.NodeAccent("Variable", "Data"), 0, 0, 0xFF, 255);
}

TEST(PaletteYaml, ColorsTakeAnOpacityOrAnAlphaChannel) {
    const palette::Palette kColors = Read(
        "node:\n"
        "  header: { color: \"#FF0000\", opacity: 0.5 }\n"
        "  body: { color: chasm.green, opacity: 0 }\n"
        "  title: \"#11223340\"\n");
    ExpectColor(kColors.node_.header_, 0xFF, 0, 0, 128);
    ExpectColor(kColors.node_.body_, 0x6E, 0xA1, 0x7D, 0);
    ExpectColor(kColors.node_.title_, 0x11, 0x22, 0x33, 0x40);
}

TEST(PaletteYaml, TablesAcceptNewNames) {
    const palette::Palette kColors = Read(
        "type_names:\n  char: chasm.green\n"
        "accent_nodes:\n  Print: \"#123456\"\n"
        "accent_categories:\n  Math: accent.variable\n");
    ExpectColor(kColors.TypeColor("char"), 0x6E, 0xA1, 0x7D, 255);
    ExpectColor(kColors.NodeAccent("Print", "IO"), 0x12, 0x34, 0x56, 255);
    ExpectColor(kColors.NodeAccent("Operator", "Math"), 0x76, 0xA1, 0xE1, 255);
    // The built-in entries stay.
    ExpectColor(kColors.TypeColor("int"), 0x76, 0xA1, 0xE1, 255);
}

TEST(PaletteYaml, UnquotedHexCodesAreReported) {
    // # starts a YAML comment: the color is empty, the default stays.
    const palette::Palette kColors = Read("canvas:\n  grid: #102030\n", 1);
    ExpectColor(kColors.canvas_.grid_, 0x2E, 0x30, 0x37, 255);
}

TEST(PaletteYaml, InvalidColorsKeepTheirDefault) {
    const palette::Palette kColors = Read(
        "canvas:\n"
        "  grid: \"#12345\"\n"
        "  background: \"#GGGGGG\"\n"
        "  loading_text: chasm.pink\n"
        "  selection_fill: { color: base.accent, opacity: high }\n"
        "  selection_border: [1, 2, 3]\n",
        5);
    const palette::Palette kDefault = palette::Palette::Default();
    EXPECT_TRUE(SameColor(kColors.canvas_.grid_, kDefault.canvas_.grid_));
    EXPECT_TRUE(
        SameColor(kColors.canvas_.background_, kDefault.canvas_.background_));
    EXPECT_TRUE(SameColor(kColors.canvas_.loading_text_,
                          kDefault.canvas_.loading_text_));
    EXPECT_TRUE(SameColor(kColors.canvas_.selection_fill_,
                          kDefault.canvas_.selection_fill_));
    EXPECT_TRUE(SameColor(kColors.canvas_.selection_border_,
                          kDefault.canvas_.selection_border_));
}

TEST(PaletteYaml, CircularReferencesKeepTheDefault) {
    const palette::Palette kColors = Read(
        "canvas:\n  grid: canvas.background\n  background: canvas.grid\n", 1);
    const palette::Palette kDefault = palette::Palette::Default();
    EXPECT_TRUE(
        SameColor(kColors.canvas_.grid_, kDefault.canvas_.grid_) ||
        SameColor(kColors.canvas_.background_, kDefault.canvas_.background_));
}

TEST(PaletteYaml, UnknownNamesAreReported) {
    static_cast<void>(
        Read("canvas:\n  sky: \"#000000\"\n"
             "toolbar:\n  text: \"#000000\"\n",
             2));
}

TEST(PaletteYaml, AnOpacityOutOfRangeIsClamped) {
    const palette::Palette kColors =
        Read("canvas:\n  grid: { color: \"#102030\", opacity: 3 }\n", 1);
    ExpectColor(kColors.canvas_.grid_, 0x10, 0x20, 0x30, 255);
}

TEST(PaletteYaml, InvalidYamlThrows) {
    std::vector<std::string> warnings;
    EXPECT_THROW(
        static_cast<void>(palette::Palette::FromYaml("canvas: [", warnings)),
        std::runtime_error);
    EXPECT_THROW(
        static_cast<void>(palette::Palette::FromYaml("- a\n- b\n", warnings)),
        std::runtime_error);
}

TEST(PaletteYaml, AMissingFileGivesTheDefaultPalette) {
    std::vector<std::string> warnings;
    const palette::Palette kColors =
        palette::Palette::Load("no/such/palette.yaml", warnings);
    EXPECT_EQ(warnings.size(), 1U);
    EXPECT_TRUE(SameColor(kColors.canvas_.grid_,
                          palette::Palette::Default().canvas_.grid_));
}

TEST(PaletteYaml, SetChangesTheColorsTheEditorDrawsWith) {
    palette::Set(Read("type_names:\n  int: \"#010203\"\n"));
    ExpectColor(palette::TypeColor("int"), 1, 2, 3, 255);
    palette::Set(palette::Palette::Default());
    ExpectColor(palette::TypeColor("int"), 0x76, 0xA1, 0xE1, 255);
}
