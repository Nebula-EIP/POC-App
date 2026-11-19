/**
 * @file component_adapter_test.cpp
 * @brief Tests of the node fields: editing, validation, callbacks, caret,
 * selection and clipboard
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 11-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 13-11-2026
 */

#include "ui/component_adapter.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

using core::capa::Checkbox;
using core::capa::ComponentList;
using core::capa::NumberField;
using core::capa::Select;
using core::capa::TextField;
using editor::ui::ComponentAdapter;
using editor::ui::ComponentInput;

constexpr core::NodeId kNode = 4;
constexpr utils::WrappedRectangle kBounds{100.0F, 50.0F, 200.0F, 96.0F};
/// Width of every character with the test measurer.
constexpr float kCharWidth = 10.0F;
/// Space between the left edge of a field and its text.
constexpr float kTextLeft = 6.0F;

using Selection = std::pair<std::size_t, std::size_t>;

float MeasureFixed(const std::string &text) {
    return kCharWidth * static_cast<float>(std::count_if(
                            text.begin(), text.end(), [](char character) {
                                return (static_cast<unsigned char>(character) &
                                        0xC0U) != 0x80U;
                            }));
}

/**
 * @brief Centre of the control of a row.
 *
 */
utils::WrappedVector2 RowCenter(const ComponentList &components,
                                std::size_t index) {
    const auto kControl =
        ComponentAdapter::ControlBounds(components, kBounds, index);
    return {kControl.x_ + kControl.width_ / 2.0F,
            kControl.y_ + kControl.height_ / 2.0F};
}

/**
 * @brief Point of a field just before a character (0 = before the first),
 * when the field is not scrolled.
 *
 */
utils::WrappedVector2 Before(const ComponentList &components, std::size_t row,
                             std::size_t character) {
    const auto kControl =
        ComponentAdapter::ControlBounds(components, kBounds, row);
    return {
        kControl.x_ + kTextLeft + kCharWidth * static_cast<float>(character),
        kControl.y_ + kControl.height_ / 2.0F};
}

ComponentInput Click(utils::WrappedVector2 cursor) {
    ComponentInput input;
    input.cursor_ = cursor;
    input.left_pressed_ = true;
    input.left_down_ = true;
    return input;
}

ComponentInput DoubleClick(utils::WrappedVector2 cursor) {
    ComponentInput input = Click(cursor);
    input.double_click_ = true;
    return input;
}

/// The button held, the cursor moved.
ComponentInput Drag(utils::WrappedVector2 cursor) {
    ComponentInput input;
    input.cursor_ = cursor;
    input.left_down_ = true;
    return input;
}

ComponentInput Release(utils::WrappedVector2 cursor) {
    ComponentInput input;
    input.cursor_ = cursor;
    return input;
}

/**
 * @brief One key pressed, the cursor away from the fields.
 *
 */
ComponentInput Press(bool ComponentInput::*key, bool shift = false,
                     bool control = false) {
    ComponentInput input;
    input.cursor_ = {-1000.0F, -1000.0F};
    input.*key = true;
    input.shift_ = shift;
    input.control_ = control;
    return input;
}

ComponentInput Type(const std::string &text) {
    ComponentInput input;
    input.cursor_ = {-1000.0F, -1000.0F};
    input.typed_ = text;
    return input;
}

ComponentInput Key(bool backspace, bool enter, bool escape) {
    ComponentInput input;
    input.cursor_ = {-1000.0F, -1000.0F};
    input.backspace_ = backspace;
    input.enter_ = enter;
    input.escape_ = escape;
    return input;
}

class ComponentAdapterTest : public ::testing::Test {
   protected:
    void SetUp() override {
        TextField name;
        name.id_ = 1;
        name.label_ = "Nom";
        name.value_ = "count";
        name.validate_ = [](const std::string &value) {
            return value.empty() ? std::string{"Le nom est vide."}
                                 : std::string{};
        };
        name.on_change_ = [this](const std::string &value) {
            texts_.push_back(value);
        };

        NumberField number;
        number.id_ = 2;
        number.label_ = "Valeur";
        number.value_ = 4.0;
        number.min_ = -10.0;
        number.max_ = 10.0;
        number.is_integer_ = true;
        number.on_change_ = [this](double value) { numbers_.push_back(value); };

        Checkbox flag;
        flag.id_ = 3;
        flag.label_ = "Actif";
        flag.value_ = false;
        flag.on_change_ = [this](bool value) { flags_.push_back(value); };

        Select select;
        select.id_ = 4;
        select.label_ = "Type";
        select.options_ = {"int", "float", "bool"};
        select.selected_index_ = 0;
        select.on_change_ = [this](std::size_t index) {
            selections_.push_back(index);
        };

        components_ = {name, number, flag, select};
    }

    bool Send(const ComponentInput &input) {
        return adapter_.Update(kNode, components_, kBounds, input);
    }

    /**
     * @brief Starts editing the name field with another value, the caret at
     * its end.
     *
     */
    void EditName(const std::string &value) {
        std::get<TextField>(components_[0]).value_ = value;
        Send(Click(RowCenter(components_, 0)));
        Send(Release(RowCenter(components_, 0)));
        Send(Press(&ComponentInput::end_));
    }

    std::string clipboard_;
    ComponentAdapter adapter_{
        MeasureFixed,
        ComponentAdapter::Clipboard{
            [this] { return clipboard_; },
            [this](const std::string &text) { clipboard_ = text; }}};
    ComponentList components_;
    std::vector<std::string> texts_;
    std::vector<double> numbers_;
    std::vector<bool> flags_;
    std::vector<std::size_t> selections_;
};

TEST_F(ComponentAdapterTest, SizesOneRowPerComponent) {
    EXPECT_EQ(ComponentAdapter::PreferredSize({}).y_, 0.0F);
    const auto kSize = ComponentAdapter::PreferredSize(components_);
    EXPECT_FLOAT_EQ(kSize.y_, 4.0F * ComponentAdapter::kRowHeight);
    EXPECT_GE(kSize.x_, 96.0F);

    // Controls share one label column and stay inside the bounds.
    const auto kFirst =
        ComponentAdapter::ControlBounds(components_, kBounds, 0);
    const auto kLast = ComponentAdapter::ControlBounds(components_, kBounds, 3);
    EXPECT_FLOAT_EQ(kFirst.x_, kLast.x_);
    EXPECT_GT(kFirst.x_, kBounds.x_);
    EXPECT_LE(kFirst.x_ + kFirst.width_, kBounds.x_ + kBounds.width_);
    EXPECT_GT(kLast.y_, kFirst.y_);
}

TEST_F(ComponentAdapterTest, ClickOnAFieldStartsEditingWithItsValue) {
    EXPECT_TRUE(Send(Click(RowCenter(components_, 0))));

    EXPECT_TRUE(adapter_.IsEditingText());
    EXPECT_EQ(adapter_.FocusedNode(), kNode);
    EXPECT_EQ(adapter_.EditedText(), "count");
    EXPECT_TRUE(texts_.empty());
}

TEST_F(ComponentAdapterTest, EnterAppliesTheTypedText) {
    Send(Click(RowCenter(components_, 0)));
    Send(Type("er"));
    EXPECT_EQ(adapter_.EditedText(), "counter");

    Send(Key(false, true, false));

    EXPECT_FALSE(adapter_.IsEditingText());
    EXPECT_EQ(texts_, std::vector<std::string>{"counter"});
}

TEST_F(ComponentAdapterTest, BackspaceRemovesAWholeCharacter) {
    Send(Click(RowCenter(components_, 0)));
    Send(Type("é"));
    Send(Key(true, false, false));
    EXPECT_EQ(adapter_.EditedText(), "count");
}

TEST_F(ComponentAdapterTest, EscapeCancelsTheEdit) {
    Send(Click(RowCenter(components_, 0)));
    Send(Type("zzz"));
    Send(Key(false, false, true));

    EXPECT_FALSE(adapter_.IsEditingText());
    EXPECT_TRUE(texts_.empty());
}

TEST_F(ComponentAdapterTest, InvalidTextShowsItsReasonAndIsNotApplied) {
    Send(Click(RowCenter(components_, 0)));
    for (int index = 0; index < 5; ++index) Send(Key(true, false, false));

    EXPECT_EQ(adapter_.EditedText(), "");
    EXPECT_EQ(adapter_.EditError(), "Le nom est vide.");

    // Enter keeps the field open while the text is invalid.
    Send(Key(false, true, false));
    EXPECT_TRUE(adapter_.IsEditingText());
    EXPECT_TRUE(texts_.empty());

    // Typing a valid text clears the reason.
    Send(Type("n"));
    EXPECT_TRUE(adapter_.EditError().empty());
}

TEST_F(ComponentAdapterTest, ClickElsewhereAppliesAValidEdit) {
    Send(Click(RowCenter(components_, 0)));
    Send(Type("2"));

    EXPECT_FALSE(Send(Click({0.0F, 0.0F})));

    EXPECT_FALSE(adapter_.IsEditingText());
    EXPECT_EQ(texts_, std::vector<std::string>{"count2"});
}

TEST_F(ComponentAdapterTest, ClickElsewhereDropsAnInvalidEditWithAMessage) {
    Send(Click(RowCenter(components_, 0)));
    for (int index = 0; index < 5; ++index) Send(Key(true, false, false));

    Send(Click({0.0F, 0.0F}));

    EXPECT_FALSE(adapter_.IsEditingText());
    EXPECT_TRUE(texts_.empty());
    EXPECT_EQ(
        adapter_.TakeMessages(),
        std::vector<std::string>{"Modification annulée : Le nom est vide."});
    EXPECT_TRUE(adapter_.TakeMessages().empty());
}

TEST_F(ComponentAdapterTest, UnchangedTextIsNotReported) {
    Send(Click(RowCenter(components_, 0)));
    Send(Key(false, true, false));
    EXPECT_TRUE(texts_.empty());
}

TEST_F(ComponentAdapterTest, ClickOnAnotherFieldAppliesAndSwitches) {
    Send(Click(RowCenter(components_, 0)));
    Send(Type("s"));

    EXPECT_TRUE(Send(Click(RowCenter(components_, 1))));

    EXPECT_EQ(texts_, std::vector<std::string>{"counts"});
    EXPECT_TRUE(adapter_.IsEditingText());
    EXPECT_EQ(adapter_.EditedText(), "4");
}

TEST_F(ComponentAdapterTest, NumberFieldParsesTheTypedNumber) {
    Send(Click(RowCenter(components_, 1)));
    Send(Key(true, false, false));
    Send(Type("-7"));
    Send(Key(false, true, false));

    EXPECT_EQ(numbers_, std::vector<double>{-7.0});
}

TEST_F(ComponentAdapterTest, NumberFieldExplainsInvalidNumbers) {
    Send(Click(RowCenter(components_, 1)));
    Send(Type("x"));
    EXPECT_EQ(adapter_.EditError(), "« 4x » n'est pas un nombre.");

    Send(Key(true, false, false));
    Send(Type(".5"));
    EXPECT_EQ(adapter_.EditError(), "La valeur doit être un nombre entier.");

    Send(Key(true, false, false));
    Send(Key(true, false, false));
    Send(Type("0"));
    EXPECT_EQ(adapter_.EditError(),
              "La valeur doit être comprise entre -10 et 10.");

    Send(Key(false, true, false));
    EXPECT_TRUE(numbers_.empty());
}

TEST(ComponentAdapterNumberTest, AcceptsACommaAsDecimalSeparator) {
    NumberField field;
    field.min_ = -100.0;
    field.max_ = 100.0;
    double value = 0.0;
    EXPECT_TRUE(ComponentAdapter::ValidateNumber("3,5", field, &value).empty());
    EXPECT_DOUBLE_EQ(value, 3.5);
    EXPECT_EQ(ComponentAdapter::ValidateNumber("", field, &value),
              "Entrez un nombre.");
    EXPECT_FALSE(
        ComponentAdapter::ValidateNumber("nan", field, &value).empty());
    EXPECT_EQ(ComponentAdapter::FormatNumber(4.0, true), "4");
    EXPECT_EQ(ComponentAdapter::FormatNumber(2.5, false), "2.5");
}

TEST_F(ComponentAdapterTest, CheckboxClickToggles) {
    EXPECT_TRUE(Send(Click(RowCenter(components_, 2))));
    EXPECT_EQ(flags_, std::vector<bool>{true});
    EXPECT_FALSE(adapter_.IsEditingText());
}

TEST_F(ComponentAdapterTest, SelectOpensAListAndPicksAnOption) {
    EXPECT_TRUE(Send(Click(RowCenter(components_, 3))));
    EXPECT_TRUE(adapter_.IsListOpen());
    EXPECT_FALSE(adapter_.IsEditingText());

    const auto kOption =
        ComponentAdapter::OptionBounds(components_, kBounds, 3, 2);
    // The list hangs below the node: the click must still be used.
    EXPECT_GT(kOption.y_, kBounds.y_ + kBounds.height_);
    EXPECT_TRUE(Send(Click({kOption.x_ + 5.0F, kOption.y_ + 5.0F})));

    EXPECT_FALSE(adapter_.IsListOpen());
    EXPECT_EQ(selections_, std::vector<std::size_t>{2});
}

TEST_F(ComponentAdapterTest, PickingTheCurrentOptionChangesNothing) {
    Send(Click(RowCenter(components_, 3)));
    const auto kOption =
        ComponentAdapter::OptionBounds(components_, kBounds, 3, 0);
    Send(Click({kOption.x_ + 5.0F, kOption.y_ + 5.0F}));
    EXPECT_TRUE(selections_.empty());
}

TEST_F(ComponentAdapterTest, ListClosesOnEscapeOrClickOutside) {
    Send(Click(RowCenter(components_, 3)));
    Send(Key(false, false, true));
    EXPECT_FALSE(adapter_.IsListOpen());

    Send(Click(RowCenter(components_, 3)));
    EXPECT_FALSE(Send(Click({0.0F, 0.0F})));
    EXPECT_FALSE(adapter_.IsListOpen());
    EXPECT_TRUE(selections_.empty());
}

TEST_F(ComponentAdapterTest, AnotherNodeDropsTheEdit) {
    Send(Click(RowCenter(components_, 0)));
    Send(Type("zzz"));

    adapter_.Update(kNode + 1, components_, kBounds, ComponentInput{});

    EXPECT_FALSE(adapter_.IsEditingText());
    EXPECT_TRUE(texts_.empty());
}

TEST_F(ComponentAdapterTest, ReadOnlyAndDisabledFieldsAreNotEdited) {
    std::get<TextField>(components_[0]).read_only_ = true;
    std::get<Checkbox>(components_[2]).enabled_ = false;

    Send(Click(RowCenter(components_, 0)));
    Send(Click(RowCenter(components_, 2)));

    EXPECT_FALSE(adapter_.IsEditingText());
    EXPECT_TRUE(flags_.empty());
}

TEST_F(ComponentAdapterTest, ClearNodeForgetsItsEdit) {
    Send(Click(RowCenter(components_, 0)));
    EXPECT_TRUE(adapter_.HasState(kNode));
    adapter_.ClearNode(kNode);
    EXPECT_FALSE(adapter_.HasState(kNode));
    EXPECT_FALSE(adapter_.IsEditingText());
}

// --- Caret and selection -------------------------------------------------

TEST_F(ComponentAdapterTest, ClickPlacesTheCaretAtTheClickedPosition) {
    EXPECT_TRUE(Send(Click(Before(components_, 0, 2))));

    EXPECT_EQ(adapter_.Caret(), 2U);
    EXPECT_EQ(adapter_.Selection(), Selection(2, 2));
    Send(Release(Before(components_, 0, 2)));
    Send(Type("X"));
    EXPECT_EQ(adapter_.EditedText(), "coXunt");
    EXPECT_EQ(adapter_.Caret(), 3U);
}

TEST_F(ComponentAdapterTest, ClickGoesToTheClosestCharacterBoundary) {
    Send(Click(Before(components_, 0, 0)));
    EXPECT_EQ(adapter_.Caret(), 0U);

    // Closer to the end of the third character than to its start.
    auto point = Before(components_, 0, 2);
    point.x_ += kCharWidth * 0.8F;
    Send(Click(point));
    EXPECT_EQ(adapter_.Caret(), 3U);

    // Past the end of the text.
    Send(Click(RowCenter(components_, 0)));
    EXPECT_EQ(adapter_.Caret(), 5U);
}

TEST_F(ComponentAdapterTest, ClickInTheEditedFieldMovesTheCaret) {
    Send(Click(RowCenter(components_, 0)));
    Send(Release(RowCenter(components_, 0)));
    EXPECT_TRUE(Send(Click(Before(components_, 0, 1))));

    EXPECT_TRUE(adapter_.IsEditingText());
    EXPECT_EQ(adapter_.Caret(), 1U);
    EXPECT_TRUE(texts_.empty());
}

TEST_F(ComponentAdapterTest, CaretCountsCharactersNotBytes) {
    std::get<TextField>(components_[0]).value_ = "héllo";
    Send(Click(Before(components_, 0, 2)));
    EXPECT_EQ(adapter_.Caret(), 2U);

    Send(Type("x"));
    EXPECT_EQ(adapter_.EditedText(), "héxllo");
    Send(Press(&ComponentInput::backspace_));
    Send(Press(&ComponentInput::backspace_));
    EXPECT_EQ(adapter_.EditedText(), "hllo");
    EXPECT_EQ(adapter_.Caret(), 1U);

    Send(Type("é"));
    Send(Press(&ComponentInput::left_, true));
    EXPECT_EQ(adapter_.SelectedText(), "é");
}

TEST_F(ComponentAdapterTest, DragSelectsCharactersAndKeepsThePointer) {
    EXPECT_TRUE(Send(Click(Before(components_, 0, 1))));
    EXPECT_TRUE(Send(Drag(Before(components_, 0, 3))));
    EXPECT_TRUE(Send(Drag(Before(components_, 0, 4))));

    EXPECT_EQ(adapter_.Selection(), Selection(1, 4));
    EXPECT_EQ(adapter_.SelectedText(), "oun");
    EXPECT_EQ(adapter_.Caret(), 4U);

    // Released: the selection stays, the pointer is free again.
    EXPECT_FALSE(Send(Release(Before(components_, 0, 4))));
    EXPECT_FALSE(Send(Drag({0.0F, 0.0F})));
    EXPECT_EQ(adapter_.Selection(), Selection(1, 4));
}

TEST_F(ComponentAdapterTest, DragTowardsTheStartSelectsToo) {
    Send(Click(Before(components_, 0, 4)));
    Send(Drag(Before(components_, 0, 1)));

    EXPECT_EQ(adapter_.Selection(), Selection(1, 4));
    EXPECT_EQ(adapter_.Caret(), 1U);
}

TEST_F(ComponentAdapterTest, DoubleClickSelectsTheWholeText) {
    Send(Click(Before(components_, 0, 2)));
    Send(Release(Before(components_, 0, 2)));
    EXPECT_TRUE(Send(DoubleClick(Before(components_, 0, 2))));

    EXPECT_EQ(adapter_.Selection(), Selection(0, 5));
    EXPECT_EQ(adapter_.SelectedText(), "count");
    // Moving the held mouse does not shrink the selection.
    Send(Drag(Before(components_, 0, 3)));
    EXPECT_EQ(adapter_.Selection(), Selection(0, 5));
}

TEST_F(ComponentAdapterTest, DoubleClickOnAnIdleFieldEditsItAllSelected) {
    Send(DoubleClick(Before(components_, 1, 0)));

    EXPECT_TRUE(adapter_.IsEditingText());
    EXPECT_EQ(adapter_.EditedText(), "4");
    EXPECT_EQ(adapter_.Selection(), Selection(0, 1));
}

TEST_F(ComponentAdapterTest, CtrlASelectsTheWholeText) {
    Send(Click(Before(components_, 0, 2)));
    Send(Press(&ComponentInput::select_all_, false, true));

    EXPECT_EQ(adapter_.Selection(), Selection(0, 5));
}

TEST_F(ComponentAdapterTest, ShiftClickExtendsTheSelection) {
    Send(Click(Before(components_, 0, 1)));
    Send(Release(Before(components_, 0, 1)));
    ComponentInput click = Click(Before(components_, 0, 3));
    click.shift_ = true;
    Send(click);

    EXPECT_EQ(adapter_.Selection(), Selection(1, 3));
}

TEST_F(ComponentAdapterTest, ArrowsHomeAndEndMoveTheCaret) {
    Send(Click(Before(components_, 0, 2)));

    Send(Press(&ComponentInput::home_));
    EXPECT_EQ(adapter_.Caret(), 0U);
    Send(Press(&ComponentInput::left_));
    EXPECT_EQ(adapter_.Caret(), 0U);
    Send(Press(&ComponentInput::right_));
    EXPECT_EQ(adapter_.Caret(), 1U);
    Send(Press(&ComponentInput::end_));
    EXPECT_EQ(adapter_.Caret(), 5U);
    Send(Press(&ComponentInput::right_));
    EXPECT_EQ(adapter_.Caret(), 5U);
    Send(Press(&ComponentInput::left_));
    EXPECT_EQ(adapter_.Caret(), 4U);
    EXPECT_EQ(adapter_.Selection(), Selection(4, 4));
}

TEST_F(ComponentAdapterTest, ShiftExtendsTheSelectionWithTheKeyboard) {
    Send(Click(Before(components_, 0, 0)));
    Send(Press(&ComponentInput::right_, true));
    Send(Press(&ComponentInput::right_, true));
    EXPECT_EQ(adapter_.Selection(), Selection(0, 2));

    // Without Shift, an arrow drops the selection at its side.
    Send(Press(&ComponentInput::right_));
    EXPECT_EQ(adapter_.Caret(), 2U);
    EXPECT_EQ(adapter_.Selection(), Selection(2, 2));

    Send(Press(&ComponentInput::end_, true));
    EXPECT_EQ(adapter_.Selection(), Selection(2, 5));
    Send(Press(&ComponentInput::left_));
    EXPECT_EQ(adapter_.Caret(), 2U);

    Send(Press(&ComponentInput::home_, true));
    EXPECT_EQ(adapter_.Selection(), Selection(0, 2));
}

TEST_F(ComponentAdapterTest, CtrlArrowsMoveByWord) {
    EditName("hello big_word, 42");

    Send(Press(&ComponentInput::left_, false, true));
    EXPECT_EQ(adapter_.Caret(), 16U);
    Send(Press(&ComponentInput::left_, false, true));
    EXPECT_EQ(adapter_.Caret(), 6U);
    Send(Press(&ComponentInput::left_, true, true));
    EXPECT_EQ(adapter_.Selection(), Selection(0, 6));

    Send(Press(&ComponentInput::right_, false, true));
    EXPECT_EQ(adapter_.Caret(), 5U);
    Send(Press(&ComponentInput::right_, false, true));
    EXPECT_EQ(adapter_.Caret(), 14U);
}

// --- Editing ---------------------------------------------------------------

TEST_F(ComponentAdapterTest, TypingReplacesTheSelection) {
    Send(Click(Before(components_, 0, 1)));
    Send(Drag(Before(components_, 0, 4)));
    Send(Release(Before(components_, 0, 4)));

    Send(Type("ab"));

    EXPECT_EQ(adapter_.EditedText(), "cabt");
    EXPECT_EQ(adapter_.Caret(), 3U);
    EXPECT_EQ(adapter_.Selection(), Selection(3, 3));
}

TEST_F(ComponentAdapterTest, BackspaceAndDeleteWorkAroundTheCaret) {
    Send(Click(Before(components_, 0, 2)));

    Send(Press(&ComponentInput::backspace_));
    EXPECT_EQ(adapter_.EditedText(), "cunt");
    EXPECT_EQ(adapter_.Caret(), 1U);

    Send(Press(&ComponentInput::delete_));
    EXPECT_EQ(adapter_.EditedText(), "cnt");
    EXPECT_EQ(adapter_.Caret(), 1U);

    Send(Press(&ComponentInput::home_));
    Send(Press(&ComponentInput::backspace_));
    EXPECT_EQ(adapter_.EditedText(), "cnt");
    Send(Press(&ComponentInput::end_));
    Send(Press(&ComponentInput::delete_));
    EXPECT_EQ(adapter_.EditedText(), "cnt");
}

TEST_F(ComponentAdapterTest, BackspaceAndDeleteRemoveTheSelection) {
    Send(Click(Before(components_, 0, 1)));
    Send(Press(&ComponentInput::right_, true));
    Send(Press(&ComponentInput::right_, true));
    Send(Press(&ComponentInput::backspace_));
    EXPECT_EQ(adapter_.EditedText(), "cnt");
    EXPECT_EQ(adapter_.Caret(), 1U);

    Send(Press(&ComponentInput::select_all_, false, true));
    Send(Press(&ComponentInput::delete_));
    EXPECT_EQ(adapter_.EditedText(), "");
    EXPECT_EQ(adapter_.EditError(), "Le nom est vide.");
}

TEST_F(ComponentAdapterTest, CtrlBackspaceAndDeleteRemoveAWord) {
    EditName("one two three");
    Send(Press(&ComponentInput::backspace_, false, true));
    EXPECT_EQ(adapter_.EditedText(), "one two ");

    Send(Press(&ComponentInput::home_));
    Send(Press(&ComponentInput::delete_, false, true));
    EXPECT_EQ(adapter_.EditedText(), " two ");
}

TEST_F(ComponentAdapterTest, EnterAppliesATextEditedInTheMiddle) {
    Send(Click(Before(components_, 0, 0)));
    Send(Type("my_"));
    Send(Press(&ComponentInput::enter_));

    EXPECT_FALSE(adapter_.IsEditingText());
    EXPECT_EQ(texts_, std::vector<std::string>{"my_count"});
}

TEST_F(ComponentAdapterTest, NumberFieldsHaveACaretToo) {
    Send(Click(Before(components_, 1, 0)));
    Send(Type("-"));
    EXPECT_EQ(adapter_.EditedText(), "-4");
    Send(Press(&ComponentInput::enter_));
    EXPECT_EQ(numbers_, std::vector<double>{-4.0});
}

// --- Clipboard -------------------------------------------------------------

TEST_F(ComponentAdapterTest, CtrlCCopiesTheSelection) {
    clipboard_ = "before";
    Send(Click(Before(components_, 0, 1)));
    Send(Press(&ComponentInput::copy_, false, true));
    EXPECT_EQ(clipboard_, "before");  // Nothing selected: nothing copied.

    Send(Press(&ComponentInput::right_, true));
    Send(Press(&ComponentInput::right_, true));
    Send(Press(&ComponentInput::copy_, false, true));

    EXPECT_EQ(clipboard_, "ou");
    EXPECT_EQ(adapter_.EditedText(), "count");
}

TEST_F(ComponentAdapterTest, CtrlXCutsTheSelection) {
    Send(Click(Before(components_, 0, 1)));
    Send(Drag(Before(components_, 0, 4)));
    Send(Release(Before(components_, 0, 4)));
    Send(Press(&ComponentInput::cut_, false, true));

    EXPECT_EQ(clipboard_, "oun");
    EXPECT_EQ(adapter_.EditedText(), "ct");
    EXPECT_EQ(adapter_.Caret(), 1U);
}

TEST_F(ComponentAdapterTest, CtrlVPastesOneLineOverTheSelection) {
    clipboard_ = "a\r\nb\n";
    Send(Click(Before(components_, 0, 1)));
    Send(Drag(Before(components_, 0, 4)));
    Send(Release(Before(components_, 0, 4)));
    Send(Press(&ComponentInput::paste_, false, true));

    EXPECT_EQ(adapter_.EditedText(), "cabt");
    EXPECT_EQ(adapter_.Caret(), 3U);
}

TEST_F(ComponentAdapterTest, TheShortcutLetterIsNotTyped) {
    clipboard_ = "X";
    Send(Click(Before(components_, 0, 5)));
    ComponentInput paste = Press(&ComponentInput::paste_, false, true);
    paste.typed_ = "v";
    Send(paste);

    EXPECT_EQ(adapter_.EditedText(), "countX");
}

// --- Scrolling ---------------------------------------------------------------

TEST_F(ComponentAdapterTest, LongTextScrollsToShowTheCaret) {
    // 30 characters, the field shows about 12 of them.
    EditName("abcdefghijklmnopqrstuvwxyz0123");
    const std::size_t kScrolled = adapter_.FirstVisibleCharacter();
    EXPECT_GT(kScrolled, 0U);
    EXPECT_EQ(adapter_.Caret(), 30U);

    Send(Press(&ComponentInput::home_));
    EXPECT_EQ(adapter_.FirstVisibleCharacter(), 0U);
    Send(Press(&ComponentInput::end_));
    EXPECT_EQ(adapter_.FirstVisibleCharacter(), kScrolled);

    // A click counts from the first visible character.
    Send(Click(Before(components_, 0, 1)));
    EXPECT_EQ(adapter_.Caret(), kScrolled + 1);
}

TEST_F(ComponentAdapterTest, SelectingPastTheEdgeScrolls) {
    EditName("abcdefghijklmnopqrstuvwxyz0123");
    Send(Press(&ComponentInput::home_));
    Send(Click(Before(components_, 0, 0)));

    const auto kControl =
        ComponentAdapter::ControlBounds(components_, kBounds, 0);
    const utils::WrappedVector2 kRight{kControl.x_ + kControl.width_ + 20.0F,
                                       kControl.y_ + 5.0F};
    for (int frame = 0; frame < 40; ++frame) {
        EXPECT_TRUE(Send(Drag(kRight)));
    }
    EXPECT_EQ(adapter_.Selection(), Selection(0, 30));
    EXPECT_GT(adapter_.FirstVisibleCharacter(), 0U);

    // Back past the left edge.
    for (int frame = 0; frame < 40; ++frame) {
        Send(Drag({kControl.x_ - 20.0F, kControl.y_ + 5.0F}));
    }
    EXPECT_EQ(adapter_.Selection(), Selection(0, 0));
    EXPECT_EQ(adapter_.FirstVisibleCharacter(), 0U);
}

TEST_F(ComponentAdapterTest, ErasingScrollsBackToTheStart) {
    EditName("abcdefghijklmnopqrstuvwxyz0123");
    EXPECT_GT(adapter_.FirstVisibleCharacter(), 0U);
    for (int index = 0; index < 20; ++index) {
        Send(Press(&ComponentInput::backspace_));
    }
    EXPECT_EQ(adapter_.FirstVisibleCharacter(), 0U);
}

}  // namespace
