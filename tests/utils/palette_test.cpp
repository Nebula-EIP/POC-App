/**
 * @file palette_test.cpp
 * @brief Unit tests for the project color palette (utils/palette.hpp).
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 05-11-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 05-11-2026
 */

#include "utils/palette.hpp"

#include <gtest/gtest.h>

namespace {

void ExpectColor(utils::WrappedColor color, unsigned char red,
                 unsigned char green, unsigned char blue, unsigned char alpha) {
    EXPECT_EQ(color.r_, red);
    EXPECT_EQ(color.g_, green);
    EXPECT_EQ(color.b_, blue);
    EXPECT_EQ(color.a_, alpha);
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

TEST(Palette, ColorsCanBeUsedAtCompileTime) {
    constexpr utils::WrappedColor kColor = palette::Hex(0x112233);
    static_assert(kColor.r_ == 0x11 && kColor.g_ == 0x22 && kColor.b_ == 0x33);
    static_assert(palette::TypeColor("int").b_ == palette::type::kInt.b_);
}

TEST(Palette, ProjectColorsMatchTheDesign) {
    ExpectColor(palette::canvas::kBackground, 0x23, 0x25, 0x2D, 255);
    ExpectColor(palette::canvas::kGrid, 0x2E, 0x30, 0x37, 255);
    ExpectColor(palette::top_bar::kBackground, 0x22, 0x23, 0x2C, 255);
    ExpectColor(palette::top_bar::kText, 0x90, 0x91, 0x95, 255);
    ExpectColor(palette::top_bar::kShadow, 0, 0, 0, 64);
}

TEST(Palette, ChasmColorsMatchTheDesign) {
    ExpectColor(palette::chasm::kSilver, 0xAE, 0xAE, 0xB4, 255);
    ExpectColor(palette::chasm::kSlate, 0x4E, 0x4F, 0x56, 255);
    ExpectColor(palette::chasm::kCharcoal, 0x24, 0x25, 0x2C, 255);
    ExpectColor(palette::chasm::kAbyss, 0x16, 0x17, 0x1D, 255);
    ExpectColor(palette::chasm::kGreen, 0x6E, 0xA1, 0x7D, 255);
    ExpectColor(palette::chasm::kRed, 0xA8, 0x47, 0x4A, 255);
    ExpectColor(palette::chasm::kViolet, 0x7E, 0x7F, 0xCD, 255);
    ExpectColor(palette::chasm::kOrange, 0xD1, 0x9D, 0x82, 255);
    ExpectColor(palette::chasm::kBlue, 0x76, 0xA1, 0xE1, 255);
}

namespace {

bool SameColor(utils::WrappedColor left, utils::WrappedColor right) {
    return left.r_ == right.r_ && left.g_ == right.g_ && left.b_ == right.b_ &&
           left.a_ == right.a_;
}

}  // namespace

TEST(Palette, TypeColorLooksTypesUpByName) {
    EXPECT_TRUE(SameColor(palette::TypeColor("int"), palette::chasm::kBlue));
    EXPECT_TRUE(
        SameColor(palette::TypeColor("float"), palette::chasm::kViolet));
    EXPECT_TRUE(
        SameColor(palette::TypeColor("string"), palette::chasm::kGreen));
    EXPECT_TRUE(SameColor(palette::TypeColor("bool"), palette::chasm::kRed));
    EXPECT_TRUE(SameColor(palette::TypeColor(palette::type::kFlowTypeName),
                          palette::type::kFlow));
}

TEST(Palette, OtherTypesAreGreyAndCustomTypesOrange) {
    EXPECT_TRUE(SameColor(palette::TypeColor("char"), palette::chasm::kSlate));
    EXPECT_TRUE(SameColor(palette::TypeColor("matrix"), palette::type::kOther));
    EXPECT_TRUE(
        SameColor(palette::TypeColor("Player", true), palette::chasm::kOrange));
}

TEST(Palette, NodeAccentFollowsTheKindOfNode) {
    // Entry points in red, branches and loops in white, variables in blue.
    EXPECT_TRUE(SameColor(palette::NodeAccent("Function", "Functions"),
                          palette::chasm::kRed));
    EXPECT_TRUE(SameColor(palette::NodeAccent("FunctionOutput", "Functions"),
                          palette::chasm::kRed));
    EXPECT_TRUE(SameColor(palette::NodeAccent("Condition", "Flow"),
                          palette::chasm::kSilver));
    EXPECT_TRUE(SameColor(palette::NodeAccent("For", "Flow/Loops"),
                          palette::chasm::kSilver));
    EXPECT_TRUE(SameColor(palette::NodeAccent("Variable", "Data"),
                          palette::chasm::kBlue));
}

TEST(Palette, NodesWithoutAccentAreTransparent) {
    EXPECT_EQ(palette::NodeAccent("Literal", "Data").a_, 0);
    EXPECT_EQ(palette::NodeAccent("Operator", "Math").a_, 0);
    EXPECT_EQ(palette::NodeAccent("", "").a_, 0);
    // A folder only starting like a known one is not that one.
    EXPECT_EQ(palette::NodeAccent("Rose", "Flowers").a_, 0);
}
