/**
 * @file link_drop_test.cpp
 * @brief Tests for a link dropped on empty canvas: the nodes offered by the
 * picker (direction, type, module, one entry per pin) and the link made to
 * the picked node.
 *
 * Uses the real C module, whose nodes are Variable (out int), Literal (out
 * int), Operator (left int, right int -> result int), Condition (cond bool ->
 * true void, false void), Print (control void, value string), For (init int,
 * cond bool, step int -> body void), ...
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 24-10-2026
 */

#include "ui/link_drop.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "graph/graph.hpp"
#include "modules/loader.hpp"
#include "modules/module.hpp"
#include "node_catalog/node_catalog.hpp"

namespace {

using editor::ui::DraggedPin;
using editor::ui::HitPart;
using editor::ui::HitResult;
using editor::ui::NodePickerEntry;
using editor::ui::PickerPin;

class LinkDropTest : public ::testing::Test {
   protected:
    void SetUp() override {
        module_id_ = loader_.Load(NEBULA_C_MODULE);
        module_ = loader_.Module(module_id_);
        // Same registration as the editor: type ids from 1.
        core::DataType type_id = 1;
        while (module_->Types()->RegisterType(type_id) != nullptr) {
            ++type_id;
        }
        catalog_.Update(loader_);
    }

    void TearDown() override { loader_.UnloadAll(); }

    core::NodeDescriptor Descriptor(const std::string &name) const {
        for (const auto &node : catalog_.GetNodes()) {
            if (node.name_ == name) {
                return node;
            }
        }
        ADD_FAILURE() << name << " is not in the catalog";
        return {};
    }

    /// Creates a node the way the editor does: pins in configuration order.
    core::NodeId Create(const std::string &name) {
        const core::NodeType kType = Descriptor(name).id_;
        const auto kConfig = module_->Nodes()->GetNodeConfiguration(kType);
        core::Node &node = graph_.CreateNode(kType);
        for (const auto &pin : kConfig.input_pins_) {
            graph_.AddInputPin(node.Id(), pin.name_, pin.type_);
        }
        for (const auto &pin : kConfig.output_pins_) {
            graph_.AddOutputPin(node.Id(), pin.name_, pin.type_);
        }
        return node.Id();
    }

    /// The pin with this name on a node of the graph.
    HitResult Pin(core::NodeId node_id, const std::string &name,
                  bool is_input) const {
        const core::Node *node = graph_.GetNode(node_id);
        const auto &pins = is_input ? node->InputPins() : node->OutputPins();
        for (const auto &pin : pins) {
            if (pin.name_ == name) {
                return {.node_id_ = node_id,
                        .pin_id_ = pin.id_,
                        .part_ = is_input ? HitPart::kInputPin
                                          : HitPart::kOutputPin};
            }
        }
        ADD_FAILURE() << "no pin " << name;
        return {};
    }

    std::vector<NodePickerEntry> Entries(const HitResult &pin) {
        const auto kDragged = editor::ui::FindDraggedPin(graph_, catalog_, pin);
        EXPECT_TRUE(kDragged.has_value());
        if (!kDragged) {
            return {};
        }
        return editor::ui::LinkDropEntries(
            catalog_, *kDragged,
            [this](core::ModuleId id) {
                return std::string(loader_.Module(id)->Name());
            },
            [this](core::DataType type) {
                return std::string(module_->Types()->TypeName(type));
            });
    }

    static std::vector<std::string> Labels(
        const std::vector<NodePickerEntry> &entries) {
        std::vector<std::string> labels;
        for (const auto &entry : entries) {
            labels.push_back(entry.name_);
        }
        return labels;
    }

    static const NodePickerEntry *Find(
        const std::vector<NodePickerEntry> &entries, const std::string &node,
        const std::string &pin) {
        for (const auto &entry : entries) {
            if (entry.pin_ && entry.pin_->name_ == pin &&
                entry.search_names_.front() == node) {
                return &entry;
            }
        }
        return nullptr;
    }

    core::ModuleLoader loader_;
    core::ModuleId module_id_ = 0;
    core::IModule *module_ = nullptr;
    core::NodeCatalog catalog_;
    core::Graph graph_;
};

std::string Label(const std::string &node, const std::string &pin,
                  const std::string &type) {
    return node + " » " + pin + " (" + type + ")";
}

// ---------------------------------------------------------------------------
// Filtering

TEST_F(LinkDropTest, FromAnOutputListsTheInputsOfTheSameType) {
    const core::NodeId kLiteral = Create("Literal");
    const auto kEntries = Entries(Pin(kLiteral, "out", false));

    const auto kLabels = Labels(kEntries);
    EXPECT_EQ(kLabels, (std::vector<std::string>{
                           Label("Operator", "left", "int"),
                           Label("Operator", "right", "int"),
                           Label("For", "init", "int"),
                           Label("For", "step", "int"),
                       }));
    for (const auto &entry : kEntries) {
        ASSERT_TRUE(entry.pin_.has_value());
        EXPECT_TRUE(entry.pin_->is_input_) << entry.name_;
        EXPECT_EQ(entry.module_, "C");
        EXPECT_EQ(entry.category_, catalog_.GetNode(entry.type_)->category_);
    }
}

TEST_F(LinkDropTest, FromAnInputListsTheOutputsOfTheSameType) {
    const core::NodeId kOperator = Create("Operator");
    const auto kEntries = Entries(Pin(kOperator, "left", true));

    EXPECT_EQ(Labels(kEntries), (std::vector<std::string>{
                                    Label("Variable", "out", "int"),
                                    Label("Literal", "out", "int"),
                                    Label("Operator", "result", "int"),
                                }));
    for (const auto &entry : kEntries) {
        EXPECT_FALSE(entry.pin_->is_input_) << entry.name_;
    }
}

TEST_F(LinkDropTest, OtherTypesAreNotListed) {
    const core::NodeId kCondition = Create("Condition");
    const auto kEntries = Entries(Pin(kCondition, "cond", true));

    // Only bool outputs: there are none in the C module.
    EXPECT_TRUE(kEntries.empty()) << ::testing::PrintToString(Labels(kEntries));

    // A void output only reaches void inputs.
    const auto kFlow = Entries(Pin(kCondition, "true", false));
    ASSERT_FALSE(kFlow.empty());
    for (const auto &entry : kFlow) {
        EXPECT_TRUE(entry.pin_->is_input_);
        EXPECT_EQ(entry.pin_->type_, kFlow.front().pin_->type_);
        EXPECT_NE(entry.pin_->name_, "value") << "Print value is a string";
    }
}

TEST_F(LinkDropTest, ANodeWithSeveralPinsIsListedOncePerPin) {
    const core::NodeId kLiteral = Create("Literal");
    const auto kEntries = Entries(Pin(kLiteral, "out", false));

    const NodePickerEntry *left = Find(kEntries, "Operator", "left");
    const NodePickerEntry *right = Find(kEntries, "Operator", "right");
    ASSERT_NE(left, nullptr);
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(left->type_, right->type_);
    EXPECT_EQ(left->pin_->index_, 0U);
    EXPECT_EQ(right->pin_->index_, 1U);
    EXPECT_EQ(left->search_names_,
              (std::vector<std::string>{"Operator", "left"}));

    // For: init and step are the 1st and 3rd inputs (cond is a bool).
    EXPECT_EQ(Find(kEntries, "For", "init")->pin_->index_, 0U);
    EXPECT_EQ(Find(kEntries, "For", "step")->pin_->index_, 2U);
}

TEST_F(LinkDropTest, OnlyTheNodesOfTheDraggedModuleAreListed) {
    const core::NodeId kLiteral = Create("Literal");
    auto dragged = editor::ui::FindDraggedPin(graph_, catalog_,
                                              Pin(kLiteral, "out", false));
    ASSERT_TRUE(dragged.has_value());
    EXPECT_EQ(dragged->module_id_, module_id_);

    // Type ids are given per module: the same id elsewhere is another type.
    dragged->module_id_ = module_id_ + 1;
    EXPECT_TRUE(editor::ui::LinkDropEntries(catalog_, *dragged, {}).empty());
}

TEST_F(LinkDropTest, UnknownTypeNamesFallBackToTheTypeId) {
    const core::NodeId kLiteral = Create("Literal");
    const auto kDragged = editor::ui::FindDraggedPin(
        graph_, catalog_, Pin(kLiteral, "out", false));
    const auto kEntries = editor::ui::LinkDropEntries(catalog_, *kDragged, {});
    ASSERT_FALSE(kEntries.empty());
    EXPECT_EQ(
        kEntries.front().name_,
        Label("Operator", "left", "type " + std::to_string(kDragged->type_)));
}

TEST_F(LinkDropTest, NoDraggedPinWithoutNodeOrPin) {
    const core::NodeId kLiteral = Create("Literal");
    EXPECT_FALSE(editor::ui::FindDraggedPin(graph_, catalog_,
                                            {.node_id_ = kLiteral + 10,
                                             .pin_id_ = 1,
                                             .part_ = HitPart::kOutputPin}));
    EXPECT_FALSE(editor::ui::FindDraggedPin(
        graph_, catalog_,
        {.node_id_ = kLiteral, .pin_id_ = 9, .part_ = HitPart::kOutputPin}));
    EXPECT_FALSE(editor::ui::FindDraggedPin(
        graph_, catalog_,
        {.node_id_ = kLiteral, .pin_id_ = 1, .part_ = HitPart::kBody}));
}

// ---------------------------------------------------------------------------
// Pin of the created node

TEST_F(LinkDropTest, PickedPinMatchesTheRealPinOfTheCreatedNode) {
    const core::NodeId kLiteral = Create("Literal");
    const auto kEntries = Entries(Pin(kLiteral, "out", false));

    for (const auto &entry : kEntries) {
        const core::NodeId kNew = Create(catalog_.GetNode(entry.type_)->name_);
        const auto kResolved =
            editor::ui::ResolvePickerPin(*graph_.GetNode(kNew), *entry.pin_);
        ASSERT_TRUE(kResolved.has_value()) << entry.name_;
        const HitResult kExpected = Pin(kNew, entry.pin_->name_, true);
        EXPECT_EQ(kResolved->node_id_, kNew);
        EXPECT_EQ(kResolved->pin_id_, kExpected.pin_id_) << entry.name_;
        EXPECT_EQ(kResolved->part_, HitPart::kInputPin);
    }
}

TEST_F(LinkDropTest, PickedPinThatDoesNotMatchIsNotResolved) {
    const core::NodeId kOperator = Create("Operator");
    const core::Node &node = *graph_.GetNode(kOperator);
    const core::DataType kInt = node.InputPins().front().type_;

    EXPECT_FALSE(editor::ui::ResolvePickerPin(
        node,
        {.is_input_ = true, .index_ = 5, .name_ = "left", .type_ = kInt}));
    EXPECT_FALSE(editor::ui::ResolvePickerPin(
        node,
        {.is_input_ = true, .index_ = 0, .name_ = "right", .type_ = kInt}));
    EXPECT_FALSE(editor::ui::ResolvePickerPin(
        node, {.is_input_ = true,
               .index_ = 0,
               .name_ = "left",
               .type_ = static_cast<core::DataType>(kInt + 1)}));
}

// ---------------------------------------------------------------------------
// Link after a pick

TEST_F(LinkDropTest, PickFromAnOutputLinksItToThePickedInput) {
    const core::NodeId kLiteral = Create("Literal");
    const HitResult kDragged = Pin(kLiteral, "out", false);
    const auto kEntries = Entries(kDragged);
    const NodePickerEntry *right = Find(kEntries, "Operator", "right");
    ASSERT_NE(right, nullptr);

    const core::NodeId kNew = Create("Operator");
    const auto kOutcome =
        editor::ui::LinkNewNode(graph_, kDragged, kNew, *right->pin_);

    ASSERT_TRUE(kOutcome.Succeeded()) << kOutcome.error_;
    const auto &connections = graph_.GetAllConnections();
    ASSERT_EQ(connections.size(), 1U);
    const core::Connection &link = connections.begin()->second;
    EXPECT_EQ(link.from_node_, kLiteral);
    EXPECT_EQ(link.out_pin_, kDragged.pin_id_);
    EXPECT_EQ(link.to_node_, kNew);
    EXPECT_EQ(link.in_pin_, Pin(kNew, "right", true).pin_id_);
}

TEST_F(LinkDropTest, PickFromAnInputLinksThePickedOutputToIt) {
    const core::NodeId kOperator = Create("Operator");
    const HitResult kDragged = Pin(kOperator, "right", true);
    const auto kEntries = Entries(kDragged);
    const NodePickerEntry *literal = Find(kEntries, "Literal", "out");
    ASSERT_NE(literal, nullptr);

    const core::NodeId kNew = Create("Literal");
    const auto kOutcome =
        editor::ui::LinkNewNode(graph_, kDragged, kNew, *literal->pin_);

    ASSERT_TRUE(kOutcome.Succeeded()) << kOutcome.error_;
    ASSERT_EQ(graph_.GetAllConnections().size(), 1U);
    const core::Connection &link = graph_.GetAllConnections().begin()->second;
    EXPECT_EQ(link.from_node_, kNew);
    EXPECT_EQ(link.to_node_, kOperator);
    EXPECT_EQ(link.in_pin_, kDragged.pin_id_);
}

TEST_F(LinkDropTest, PickOnAConnectedInputReplacesItsLink) {
    const core::NodeId kOperator = Create("Operator");
    const core::NodeId kOld = Create("Literal");
    const HitResult kDragged = Pin(kOperator, "left", true);
    const core::ConnectionId kOldLink = graph_.Connect(
        kOld, Pin(kOld, "out", false).pin_id_, kOperator, kDragged.pin_id_);

    const NodePickerEntry *variable =
        Find(Entries(kDragged), "Variable", "out");
    ASSERT_NE(variable, nullptr);
    const core::NodeId kNew = Create("Variable");
    const auto kOutcome =
        editor::ui::LinkNewNode(graph_, kDragged, kNew, *variable->pin_);

    ASSERT_TRUE(kOutcome.Succeeded()) << kOutcome.error_;
    EXPECT_EQ(kOutcome.replaced_, (std::vector<core::ConnectionId>{kOldLink}));
    ASSERT_EQ(graph_.GetAllConnections().size(), 1U);
    EXPECT_EQ(graph_.GetAllConnections().begin()->second.from_node_, kNew);
}

TEST_F(LinkDropTest, RefusedLinkRemovesTheNewNode) {
    const core::NodeId kLiteral = Create("Literal");
    const HitResult kDragged = Pin(kLiteral, "out", false);
    const core::NodeId kNew = Create("Operator");
    const core::Node &node = *graph_.GetNode(kNew);

    // An output on an output: refused by the usual rules.
    const PickerPin kResult{.is_input_ = false,
                            .index_ = 0,
                            .name_ = "result",
                            .type_ = node.OutputPins().front().type_};
    const auto kOutcome =
        editor::ui::LinkNewNode(graph_, kDragged, kNew, kResult);

    EXPECT_FALSE(kOutcome.Succeeded());
    EXPECT_EQ(kOutcome.error_,
              "Impossible de relier deux sorties : reliez une sortie à une "
              "entrée.");
    ASSERT_TRUE(kOutcome.faulty_pin_.has_value());
    EXPECT_EQ(kOutcome.faulty_pin_->node_id_, kLiteral);
    EXPECT_FALSE(graph_.HasNode(kNew));
    EXPECT_TRUE(graph_.HasNode(kLiteral));
    EXPECT_TRUE(graph_.GetAllConnections().empty());
}

TEST_F(LinkDropTest, MissingPickedPinRemovesTheNewNode) {
    const core::NodeId kLiteral = Create("Literal");
    const core::NodeId kNew = Create("Variable");  // no input at all
    const PickerPin kMissing{
        .is_input_ = true, .index_ = 0, .name_ = "left", .type_ = 1};

    const auto kOutcome = editor::ui::LinkNewNode(
        graph_, Pin(kLiteral, "out", false), kNew, kMissing);

    EXPECT_FALSE(kOutcome.Succeeded());
    EXPECT_FALSE(kOutcome.error_.empty());
    EXPECT_FALSE(graph_.HasNode(kNew));
}

// ---------------------------------------------------------------------------
// Shared rules

TEST(ConnectionRuleTest, MatchesTheRulesOfAManualLink) {
    using editor::ui::CheckConnectionRule;
    using editor::ui::ConnectionRule;
    using editor::ui::PinEnd;
    const PinEnd kOut{.node_ = 1, .part_ = HitPart::kOutputPin, .type_ = 1};
    const PinEnd kIn{.node_ = 2, .part_ = HitPart::kInputPin, .type_ = 1};

    EXPECT_EQ(CheckConnectionRule(kOut, kIn), ConnectionRule::kAllowed);
    EXPECT_EQ(CheckConnectionRule(kIn, kOut), ConnectionRule::kAllowed);
    EXPECT_EQ(CheckConnectionRule(kOut, kOut), ConnectionRule::kSameDirection);
    EXPECT_EQ(CheckConnectionRule(
                  kOut, {.node_ = 1, .part_ = HitPart::kInputPin, .type_ = 1}),
              ConnectionRule::kSameNode);
    EXPECT_EQ(CheckConnectionRule(
                  kOut, {.node_ = 2, .part_ = HitPart::kInputPin, .type_ = 2}),
              ConnectionRule::kTypeMismatch);
    EXPECT_EQ(CheckConnectionRule(
                  kOut, {.node_ = 2, .part_ = HitPart::kBody, .type_ = 1}),
              ConnectionRule::kNotAPin);
    // A node not created yet is never the dragged node.
    EXPECT_EQ(CheckConnectionRule(kOut, {.node_ = std::nullopt,
                                         .part_ = HitPart::kInputPin,
                                         .type_ = 1}),
              ConnectionRule::kAllowed);
}

}  // namespace
