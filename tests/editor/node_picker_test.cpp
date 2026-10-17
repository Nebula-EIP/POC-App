/**
 * @file node_picker_test.cpp
 * @brief Unit tests for the NodePicker class: tree of modules and categories,
 * flat search, keyboard and mouse picking, closing.
 *
 * No window is opened: input is fed through NodePickerInput and text is
 * measured with a fixed width per character.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 11-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 12-10-2026
 */

#include "ui/node_picker.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "modules/loader.hpp"
#include "node_catalog/node_catalog.hpp"

namespace {

using editor::ui::NodePicker;
using editor::ui::NodePickerEntry;
using editor::ui::NodePickerInput;
using RowKind = editor::ui::NodePicker::RowKind;
using utils::WrappedRectangle;
using utils::WrappedVector2;

constexpr float kScreenWidth = 800.0F;
constexpr float kScreenHeight = 600.0F;
constexpr WrappedVector2 kOpenAt{100.0F, 100.0F};
constexpr WrappedVector2 kOutside{750.0F, 580.0F};

float FixedWidth(const std::string &text, int /*font_size*/) {
    return 8.0F * static_cast<float>(text.size());
}

WrappedVector2 Center(WrappedRectangle rectangle) {
    return {rectangle.x_ + rectangle.width_ / 2.0F,
            rectangle.y_ + rectangle.height_ / 2.0F};
}

NodePickerInput Idle(WrappedVector2 cursor = kOutside) {
    NodePickerInput input;
    input.cursor_ = cursor;
    input.screen_width_ = kScreenWidth;
    input.screen_height_ = kScreenHeight;
    return input;
}

NodePickerInput Type(std::string text) {
    NodePickerInput input = Idle();
    input.typed_ = std::move(text);
    return input;
}

NodePickerInput Click(WrappedVector2 cursor) {
    NodePickerInput input = Idle(cursor);
    input.left_pressed_ = true;
    return input;
}

NodePickerInput Key(bool NodePickerInput::*key) {
    NodePickerInput input = Idle();
    input.*key = true;
    return input;
}

std::vector<NodePickerEntry> SampleEntries() {
    // Modules and categories interleaved on purpose: the picker groups them.
    return {
        {1, "Variable", "C", "Data"},
        {2, "Loop", "C", "Flow/Loops"},
        {10, "Draw", "Graphics", ""},
        {3, "Condition", "C", "Flow"},
        {4, "For", "C", "Flow/Loops"},
        {11, "Clear", "Graphics", "/"},  // empty parts are ignored
        {5, "Literal", "C", "Data"},
    };
}

class NodePickerTest : public ::testing::Test {
   protected:
    NodePicker picker_{{}, FixedWidth};
    std::vector<core::NodeType> picked_;

    void SetUp() override { picker_.SetEntries(SampleEntries()); }

    void OpenOnly() {
        picker_.Open(kOpenAt, kScreenWidth, kScreenHeight,
                     [this](core::NodeType type) { picked_.push_back(type); });
    }

    /// Opens, then runs the first frame, which never takes typed characters.
    void Open() {
        OpenOnly();
        picker_.Update(Idle());
    }

    std::vector<std::string> Labels() const {
        std::vector<std::string> labels;
        for (const auto &row : picker_.Rows()) {
            labels.push_back(row.label_);
        }
        return labels;
    }

    std::size_t RowOf(const std::string &label) const {
        for (std::size_t row = 0; row < picker_.Rows().size(); ++row) {
            if (picker_.Rows()[row].label_ == label) {
                return row;
            }
        }
        ADD_FAILURE() << label << " is not listed";
        return 0;
    }

    std::string SelectedLabel() const {
        return picker_.SelectedRow()
                   ? picker_.Rows()[*picker_.SelectedRow()].label_
                   : "";
    }

    void ClickRow(const std::string &label) {
        const auto kRect = picker_.RowRect(RowOf(label));
        ASSERT_TRUE(kRect.has_value()) << label << " is scrolled out";
        picker_.Update(Click(Center(*kRect)));
    }
};

// ---------------------------------------------------------------------------
// Tree

TEST_F(NodePickerTest, OpensWithModuleFoldersClosed) {
    Open();
    EXPECT_TRUE(picker_.IsOpen());
    EXPECT_FALSE(picker_.IsSearching());
    const std::vector<std::string> kExpected = {"C", "Graphics"};
    EXPECT_EQ(Labels(), kExpected);
    EXPECT_EQ(picker_.Rows()[0].kind_, RowKind::kFolder);
    EXPECT_FALSE(picker_.Rows()[0].expanded_);
    EXPECT_EQ(SelectedLabel(), "C");
}

TEST_F(NodePickerTest, SingleModuleStartsOpen) {
    picker_.SetEntries({{1, "Variable", "C", "Data"}, {2, "Print", "C", ""}});
    Open();
    const std::vector<std::string> kExpected = {"C", "Data", "Print"};
    EXPECT_EQ(Labels(), kExpected);
    EXPECT_TRUE(picker_.IsExpanded("C"));
    EXPECT_FALSE(picker_.IsExpanded("C/Data"));
}

TEST_F(NodePickerTest, ClickOnFolderOpensItWithSubFoldersFirst) {
    Open();
    ClickRow("C");
    const std::vector<std::string> kExpected = {"C", "Data", "Flow",
                                                "Graphics"};
    EXPECT_EQ(Labels(), kExpected);
    EXPECT_TRUE(picker_.IsExpanded("C"));
    EXPECT_TRUE(picked_.empty());
    EXPECT_TRUE(picker_.IsOpen());

    // Sub-folders first, then the nodes directly in the folder.
    ClickRow("Flow");
    const std::vector<std::string> kFlow = {"C",     "Data",      "Flow",
                                            "Loops", "Condition", "Graphics"};
    EXPECT_EQ(Labels(), kFlow);
    EXPECT_EQ(picker_.Rows()[RowOf("Loops")].depth_, 2U);
    EXPECT_EQ(picker_.Rows()[RowOf("Condition")].depth_, 2U);
    EXPECT_EQ(picker_.Rows()[RowOf("Loops")].path_, "C/Flow/Loops");
}

TEST_F(NodePickerTest, SecondClickClosesFolder) {
    Open();
    ClickRow("C");
    ClickRow("C");
    const std::vector<std::string> kExpected = {"C", "Graphics"};
    EXPECT_EQ(Labels(), kExpected);
}

TEST_F(NodePickerTest, NodesWithoutCategorySitInTheModuleFolder) {
    Open();
    ClickRow("Graphics");
    const std::vector<std::string> kExpected = {"C", "Graphics", "Draw",
                                                "Clear"};
    EXPECT_EQ(Labels(), kExpected);
    EXPECT_EQ(picker_.Rows()[RowOf("Draw")].kind_, RowKind::kEntry);
    EXPECT_EQ(picker_.Rows()[RowOf("Draw")].depth_, 1U);
}

TEST_F(NodePickerTest, ReopeningClosesFoldersAndClearsSearch) {
    Open();
    ClickRow("C");
    picker_.Update(Type("lo"));
    picker_.Close();
    Open();
    EXPECT_TRUE(picker_.Query().empty());
    EXPECT_FALSE(picker_.IsExpanded("C"));
    EXPECT_EQ(picker_.Rows().size(), 2U);
}

// ---------------------------------------------------------------------------
// Keyboard in the tree

TEST_F(NodePickerTest, RightOpensAndEntersLeftClosesAndGoesUp) {
    Open();
    picker_.Update(Key(&NodePickerInput::right_key_pressed_));  // open C
    EXPECT_TRUE(picker_.IsExpanded("C"));
    EXPECT_EQ(SelectedLabel(), "C");

    picker_.Update(Key(&NodePickerInput::right_key_pressed_));  // into C
    EXPECT_EQ(SelectedLabel(), "Data");
    picker_.Update(Key(&NodePickerInput::right_key_pressed_));  // open Data
    picker_.Update(Key(&NodePickerInput::right_key_pressed_));  // into Data
    EXPECT_EQ(SelectedLabel(), "Variable");

    picker_.Update(Key(&NodePickerInput::left_key_pressed_));  // up to Data
    EXPECT_EQ(SelectedLabel(), "Data");
    picker_.Update(Key(&NodePickerInput::left_key_pressed_));  // close Data
    EXPECT_FALSE(picker_.IsExpanded("C/Data"));
    picker_.Update(Key(&NodePickerInput::left_key_pressed_));  // up to C
    EXPECT_EQ(SelectedLabel(), "C");
}

TEST_F(NodePickerTest, EnterTogglesFolderAndPicksNode) {
    Open();
    picker_.Update(Key(&NodePickerInput::enter_pressed_));  // open C
    EXPECT_TRUE(picker_.IsExpanded("C"));
    picker_.Update(Key(&NodePickerInput::down_pressed_));   // Data
    picker_.Update(Key(&NodePickerInput::enter_pressed_));  // open Data
    picker_.Update(Key(&NodePickerInput::down_pressed_));   // Variable
    picker_.Update(Key(&NodePickerInput::down_pressed_));   // Literal
    EXPECT_EQ(SelectedLabel(), "Literal");
    EXPECT_TRUE(picker_.IsOpen());

    picker_.Update(Key(&NodePickerInput::enter_pressed_));
    EXPECT_EQ(picked_, std::vector<core::NodeType>{5});
    EXPECT_FALSE(picker_.IsOpen());
}

TEST_F(NodePickerTest, ArrowsStopAtEnds) {
    Open();
    picker_.Update(Key(&NodePickerInput::up_pressed_));
    EXPECT_EQ(SelectedLabel(), "C");
    picker_.Update(Key(&NodePickerInput::down_pressed_));
    picker_.Update(Key(&NodePickerInput::down_pressed_));
    EXPECT_EQ(SelectedLabel(), "Graphics");
}

// ---------------------------------------------------------------------------
// Search

TEST_F(NodePickerTest, SearchShowsFlatListWithoutFolders) {
    Open();
    EXPECT_TRUE(picker_.Update(Type("O")));
    EXPECT_TRUE(picker_.IsSearching());
    // Closed folders do not hide matches; names starting with the search
    // come first.
    const std::vector<std::string> kExpected = {"Loop", "Condition", "For"};
    EXPECT_EQ(Labels(), kExpected);
    for (const auto &row : picker_.Rows()) {
        EXPECT_EQ(row.kind_, RowKind::kEntry);
        EXPECT_EQ(row.depth_, 0U);
    }
    EXPECT_EQ(SelectedLabel(), "Loop");
}

TEST_F(NodePickerTest, SearchIsCaseInsensitiveAndIgnoresFolderNames) {
    Open();
    picker_.Update(Type("LiT"));
    EXPECT_EQ(Labels(), std::vector<std::string>{"Literal"});

    picker_.Close();
    Open();
    picker_.Update(Type("Graphics"));
    ASSERT_EQ(picker_.Rows().size(), 1U);
    EXPECT_EQ(picker_.Rows()[0].kind_, RowKind::kEmpty);
    EXPECT_FALSE(picker_.SelectedRow().has_value());
}

TEST_F(NodePickerTest, ClearingSearchBringsTheTreeBack) {
    Open();
    ClickRow("C");
    picker_.Update(Type("x"));
    NodePickerInput backspace = Idle();
    backspace.backspace_pressed_ = true;
    picker_.Update(backspace);
    EXPECT_FALSE(picker_.IsSearching());
    // The folders opened before the search are still open.
    const std::vector<std::string> kExpected = {"C", "Data", "Flow",
                                                "Graphics"};
    EXPECT_EQ(Labels(), kExpected);
}

TEST_F(NodePickerTest, EnterPicksFirstSearchResult) {
    Open();
    picker_.Update(Type("clear"));
    EXPECT_TRUE(picker_.Update(Key(&NodePickerInput::enter_pressed_)));
    EXPECT_EQ(picked_, std::vector<core::NodeType>{11});
    EXPECT_FALSE(picker_.IsOpen());
}

TEST_F(NodePickerTest, LeftRightDoNothingWhileSearching) {
    Open();
    picker_.Update(Type("o"));
    picker_.Update(Key(&NodePickerInput::right_key_pressed_));
    picker_.Update(Key(&NodePickerInput::left_key_pressed_));
    EXPECT_EQ(SelectedLabel(), "Loop");
    EXPECT_FALSE(picker_.IsExpanded("C"));
}

TEST_F(NodePickerTest, ShortcutCharacterIsNotTyped) {
    OpenOnly();
    // Shift+A opened the picker: its 'A' arrives late and repeats while held.
    NodePickerInput held = Type("A");
    held.opening_key_down_ = true;
    picker_.Update(held);
    picker_.Update(held);
    EXPECT_TRUE(picker_.Query().empty());
    EXPECT_FALSE(picker_.IsSearching());

    // Released in the same frame as a late character: still not typed.
    picker_.Update(Type("A"));
    EXPECT_TRUE(picker_.Query().empty());

    // Typing works from then on, the 'a' key included.
    picker_.Update(Type("a"));
    EXPECT_EQ(picker_.Query(), "a");
    NodePickerInput again = Type("b");
    again.opening_key_down_ = true;  // 'a' held again later: typing stays on
    picker_.Update(again);
    EXPECT_EQ(picker_.Query(), "ab");
}

TEST_F(NodePickerTest, FirstFrameAfterOpeningIgnoresTyping) {
    OpenOnly();
    picker_.Update(Type("x"));
    EXPECT_TRUE(picker_.Query().empty());
    picker_.Update(Type("x"));
    EXPECT_EQ(picker_.Query(), "x");
}

TEST_F(NodePickerTest, BackspaceStillWorksWhileOpeningKeyIsHeld) {
    Open();
    picker_.Update(Type("ab"));
    NodePickerInput input = Idle();
    input.backspace_pressed_ = true;
    input.opening_key_down_ = true;
    picker_.Update(input);
    EXPECT_EQ(picker_.Query(), "a");
}

// ---------------------------------------------------------------------------
// Mouse

TEST_F(NodePickerTest, ClickOnNodePicksIt) {
    Open();
    ClickRow("Graphics");
    ClickRow("Draw");
    EXPECT_EQ(picked_, std::vector<core::NodeType>{10});
    EXPECT_FALSE(picker_.IsOpen());
}

TEST_F(NodePickerTest, SameNodeCanBePickedSeveralTimes) {
    for (int round = 0; round < 2; ++round) {
        Open();
        picker_.Update(Type("for"));
        picker_.Update(Key(&NodePickerInput::enter_pressed_));
    }
    EXPECT_EQ(picked_, (std::vector<core::NodeType>{4, 4}));
}

TEST_F(NodePickerTest, HoverSelectsOnlyWhenTheCursorMoves) {
    OpenOnly();
    const WrappedVector2 kOnGraphics =
        Center(*picker_.RowRect(RowOf("Graphics")));
    // The cursor that opened the picker does not select what is under it.
    picker_.Update(Idle(kOnGraphics));
    EXPECT_EQ(SelectedLabel(), "C");

    picker_.Update(Idle(Center(*picker_.RowRect(RowOf("C")))));
    EXPECT_EQ(SelectedLabel(), "C");
    picker_.Update(Idle(kOnGraphics));
    EXPECT_EQ(SelectedLabel(), "Graphics");

    // Keyboard moves the selection while the cursor stays still.
    NodePickerInput up = Idle(kOnGraphics);
    up.up_pressed_ = true;
    picker_.Update(up);
    EXPECT_EQ(SelectedLabel(), "C");
    picker_.Update(Idle(kOnGraphics));
    EXPECT_EQ(SelectedLabel(), "C");
}

// ---------------------------------------------------------------------------
// Closing

TEST_F(NodePickerTest, ClosedPickerIgnoresInput) {
    EXPECT_FALSE(picker_.Update(Type("abc")));
    EXPECT_TRUE(picker_.Query().empty());
}

TEST_F(NodePickerTest, EscapeClosesWithoutCreating) {
    Open();
    EXPECT_TRUE(picker_.Update(Key(&NodePickerInput::escape_pressed_)));
    EXPECT_FALSE(picker_.IsOpen());
    EXPECT_TRUE(picked_.empty());
}

TEST_F(NodePickerTest, ClickOutsideClosesAndLetsTheClickThrough) {
    Open();
    EXPECT_FALSE(picker_.Update(Click(kOutside)));
    EXPECT_FALSE(picker_.IsOpen());
    EXPECT_TRUE(picked_.empty());
}

TEST_F(NodePickerTest, OpenPickerCapturesInput) {
    Open();
    EXPECT_TRUE(picker_.Update(Idle()));
}

// ---------------------------------------------------------------------------
// Layout

TEST_F(NodePickerTest, StaysOnScreen) {
    picker_.Open({kScreenWidth - 10.0F, kScreenHeight - 10.0F}, kScreenWidth,
                 kScreenHeight, {});
    const WrappedRectangle kBounds = picker_.Bounds();
    EXPECT_LE(kBounds.x_ + kBounds.width_, kScreenWidth);
    EXPECT_LE(kBounds.y_ + kBounds.height_, kScreenHeight);
    EXPECT_GE(kBounds.x_, 0.0F);
    EXPECT_GE(kBounds.y_, 0.0F);
}

TEST_F(NodePickerTest, SizeDoesNotChangeWhenFoldersOpen) {
    Open();
    const WrappedRectangle kBefore = picker_.Bounds();
    ClickRow("C");
    ClickRow("Flow");
    const WrappedRectangle kAfter = picker_.Bounds();
    EXPECT_EQ(kBefore.width_, kAfter.width_);
    EXPECT_EQ(kBefore.height_, kAfter.height_);
}

TEST_F(NodePickerTest, LongListScrollsToKeepSelectionVisible) {
    std::vector<NodePickerEntry> many;
    for (core::NodeType type = 1; type <= 30; ++type) {
        many.push_back({type, "Node" + std::to_string(type), "M", ""});
    }
    picker_.SetEntries(many);
    Open();  // A single module folder starts open.
    ASSERT_LT(picker_.VisibleRowCount(), picker_.Rows().size());

    for (int step = 0; step < 30; ++step) {
        picker_.Update(Key(&NodePickerInput::down_pressed_));
    }
    const std::size_t kSelected = *picker_.SelectedRow();
    EXPECT_EQ(picker_.Rows()[kSelected].label_, "Node30");
    EXPECT_TRUE(picker_.RowRect(kSelected).has_value());
    EXPECT_GT(picker_.Scroll(), 0U);

    // The wheel scrolls back up when the cursor is over the picker.
    NodePickerInput wheel = Idle(Center(picker_.Bounds()));
    wheel.wheel_ = 1.0F;
    const std::size_t kBefore = picker_.Scroll();
    picker_.Update(wheel);
    EXPECT_EQ(picker_.Scroll(), kBefore - 1);
}

// ---------------------------------------------------------------------------
// Catalog

TEST(NodePickerCatalogTest, EntriesComeFromTheCatalog) {
    core::ModuleLoader loader;
    const core::ModuleId kId = loader.Load(NEBULA_C_MODULE);
    core::NodeCatalog catalog;
    catalog.Update(loader);

    const auto kEntries =
        editor::ui::EntriesFromCatalog(catalog, [&loader](core::ModuleId id) {
            return std::string(loader.Module(id)->Name());
        });
    ASSERT_EQ(kEntries.size(), catalog.GetNodes().size());
    ASSERT_FALSE(kEntries.empty());
    for (const auto &entry : kEntries) {
        EXPECT_EQ(entry.module_, "C");
        EXPECT_FALSE(entry.category_.empty()) << entry.name_;
        const auto kNode = catalog.GetNode(entry.type_);
        ASSERT_TRUE(kNode.has_value());
        EXPECT_EQ(kNode->name_, entry.name_);
        EXPECT_EQ(kNode->category_, entry.category_);
        EXPECT_EQ(kNode->module_id_, kId);
    }
}

}  // namespace
