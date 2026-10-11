/**
 * @file component_adapter_test.cpp
 * @brief Tests of the node fields: editing, validation and callbacks
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 11-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 11-10-2026
 */

#include "ui/component_adapter.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <string>
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

/// Centre of the control of a row.
utils::WrappedVector2 RowCenter(const ComponentList &components,
                                std::size_t index) {
    const auto kControl =
        ComponentAdapter::ControlBounds(components, kBounds, index);
    return {kControl.x_ + kControl.width_ / 2.0F,
            kControl.y_ + kControl.height_ / 2.0F};
}

ComponentInput Click(utils::WrappedVector2 cursor) {
    ComponentInput input;
    input.cursor_ = cursor;
    input.left_pressed_ = true;
    input.left_down_ = true;
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

    ComponentAdapter adapter_;
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

}  // namespace
