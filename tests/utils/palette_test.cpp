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

TEST(Palette, TypeColorLooksTypesUpByName) {
    const utils::WrappedColor kInt = palette::TypeColor("int");
    EXPECT_EQ(kInt.r_, palette::type::kInt.r_);
    EXPECT_EQ(kInt.g_, palette::type::kInt.g_);
    EXPECT_EQ(kInt.b_, palette::type::kInt.b_);

    const utils::WrappedColor kFlow =
        palette::TypeColor(palette::type::kFlowTypeName);
    EXPECT_EQ(kFlow.r_, palette::type::kFlow.r_);

    const utils::WrappedColor kUnknown = palette::TypeColor("matrix");
    EXPECT_EQ(kUnknown.r_, palette::type::kUnknown.r_);
    EXPECT_EQ(kUnknown.g_, palette::type::kUnknown.g_);
    EXPECT_EQ(kUnknown.b_, palette::type::kUnknown.b_);
}

TEST(Palette, CategoryAccentUsesTheRootFolder) {
    EXPECT_EQ(palette::CategoryAccent("Flow").r_, palette::accent::kFlow.r_);
    EXPECT_EQ(palette::CategoryAccent("Flow/Loops").r_,
              palette::accent::kFlow.r_);
    EXPECT_EQ(palette::CategoryAccent("Functions").r_,
              palette::accent::kFunction.r_);
}

TEST(Palette, CategoryWithoutAccentIsTransparent) {
    EXPECT_EQ(palette::CategoryAccent("Data").a_, 0);
    EXPECT_EQ(palette::CategoryAccent("").a_, 0);
    // A folder only starting like a known one is not that one.
    EXPECT_EQ(palette::CategoryAccent("Flowers").a_, 0);
}
