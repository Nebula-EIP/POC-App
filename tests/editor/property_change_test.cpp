/**
 * @file property_change_test.cpp
 * @brief Tests of the application of field changes to the graph
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 11-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 11-10-2026
 */

#include "ui/property_change.hpp"

#include <gtest/gtest.h>

#include <any>
#include <stdexcept>
#include <string>

#include "graph/graph.hpp"

namespace {

using editor::ui::ApplyPropertyChange;
using editor::ui::PropertyChangeOutcome;

constexpr core::DataType kInt = 2;
constexpr core::DataType kFloat = 3;
constexpr core::NodeType kTypedNode = 1;
constexpr core::NodeType kPlainNode = 2;
constexpr core::NodeType kBrokenNode = 3;
constexpr core::PropertyId kTypeProperty = 5;
constexpr core::PropertyId kValueProperty = 2;

/// A typed node outputs the type in its property 5 (a DataType).
class FakeNodes final : public core::capa::INodeListCapability {
   public:
    const std::string_view *RegisterNode(
        core::NodeType) const noexcept override {
        return nullptr;
    }
    std::vector<core::capa::NodeMetadata> GetAvailableNodes()
        const noexcept override {
        return {};
    }
    void InitializePropertyTypes(
        const std::unordered_map<std::string, core::PropertyTypeId> &)
        override {}
    core::capa::NodeConfiguration GetNodeConfiguration(
        core::NodeType type) const override {
        if (type == kBrokenNode) throw std::runtime_error("unknown");
        core::capa::NodeConfiguration config;
        config.input_pins_.push_back({1, "in", kInt});
        config.output_pins_.push_back({1, "out", kInt});
        return config;
    }
    core::capa::NodeConfiguration ResolveNodeConfiguration(
        core::NodeType type,
        const core::PropertyMap &properties) const override {
        core::capa::NodeConfiguration config = GetNodeConfiguration(type);
        if (type != kTypedNode) return config;
        const auto kProperty = properties.find(kTypeProperty);
        if (kProperty != properties.end()) {
            config.output_pins_[0].type_ =
                std::any_cast<core::DataType>(kProperty->second.value_);
        }
        return config;
    }
};

class PropertyChangeTest : public ::testing::Test {
   protected:
    void SetUp() override {
        literal_ = graph_.CreateNode(kTypedNode).Id();
        graph_.AddInputPin(literal_, "in", kInt);
        literal_out_ = graph_.AddOutputPin(literal_, "out", kInt);
        graph_.GetNode(literal_)->SetProperty(kTypeProperty,
                                              {.type_id_ = 0, .value_ = kInt});
        graph_.GetNode(literal_)->SetProperty(kValueProperty,
                                              {.type_id_ = 0, .value_ = 4.0});

        first_ = graph_.CreateNode(kPlainNode).Id();
        first_in_ = graph_.AddInputPin(first_, "left", kInt);
        second_ = graph_.CreateNode(kPlainNode).Id();
        second_in_ = graph_.AddInputPin(second_, "right", kInt);
    }

    PropertyChangeOutcome Apply(core::NodeId node,
                                const core::PropertyMap &changes,
                                const core::capa::INodeListCapability *nodes) {
        return ApplyPropertyChange(
            graph_, nodes, node, changes,
            [](core::DataType type) {
                return type == kInt ? std::string{"int"} : std::string{"float"};
            },
            [this](core::NodeId id) {
                return id == first_ ? std::string{"Operator"}
                                    : std::string{"Print"};
            });
    }

    core::PropertyMap ToFloat() const {
        return {{kTypeProperty, {.type_id_ = 0, .value_ = kFloat}},
                {kValueProperty, {.type_id_ = 0, .value_ = 4.5}}};
    }

    double Value() const {
        return std::any_cast<double>(
            graph_.GetNode(literal_)->GetProperty(kValueProperty)->value_);
    }

    FakeNodes nodes_;
    core::Graph graph_;
    core::NodeId literal_ = 0;
    core::PinId literal_out_ = 0;
    core::NodeId first_ = 0;
    core::PinId first_in_ = 0;
    core::NodeId second_ = 0;
    core::PinId second_in_ = 0;
};

TEST_F(PropertyChangeTest, SetsTheValueInTheGraph) {
    const auto kOutcome = Apply(
        literal_, {{kValueProperty, {.type_id_ = 0, .value_ = 12.0}}}, &nodes_);

    EXPECT_TRUE(kOutcome.applied_);
    EXPECT_TRUE(kOutcome.pin_changes_.empty());
    EXPECT_TRUE(kOutcome.error_.empty());
    EXPECT_DOUBLE_EQ(Value(), 12.0);
}

TEST_F(PropertyChangeTest, ChangesTheTypeOfAFreePin) {
    const auto kOutcome = Apply(literal_, ToFloat(), &nodes_);

    ASSERT_TRUE(kOutcome.applied_);
    ASSERT_EQ(kOutcome.pin_changes_.size(), 1U);
    EXPECT_EQ(kOutcome.pin_changes_[0].pin_id_, literal_out_);
    EXPECT_FALSE(kOutcome.pin_changes_[0].is_input_);
    EXPECT_EQ(kOutcome.pin_changes_[0].from_, kInt);
    EXPECT_EQ(kOutcome.pin_changes_[0].to_, kFloat);
    EXPECT_EQ(graph_.GetNode(literal_)->OutputPin(literal_out_)->type_, kFloat);
    EXPECT_DOUBLE_EQ(Value(), 4.5);
}

TEST_F(PropertyChangeTest, RefusesATypeChangeOnAConnectedPin) {
    const core::ConnectionId kLink =
        graph_.Connect(literal_, literal_out_, first_, first_in_);

    const auto kOutcome = Apply(literal_, ToFloat(), &nodes_);

    EXPECT_FALSE(kOutcome.applied_);
    EXPECT_EQ(kOutcome.blocking_connections_,
              std::vector<core::ConnectionId>{kLink});
    ASSERT_TRUE(kOutcome.blocked_pin_.has_value());
    EXPECT_EQ(kOutcome.blocked_pin_->pin_id_, literal_out_);
    EXPECT_EQ(kOutcome.error_,
              "Impossible de passer la sortie « out » de int à float : elle "
              "est reliée à « Operator ». Supprimez d'abord cette liaison.");
    // Nothing changed: value, pin type and link.
    EXPECT_DOUBLE_EQ(Value(), 4.0);
    EXPECT_EQ(graph_.GetNode(literal_)->OutputPin(literal_out_)->type_, kInt);
    EXPECT_EQ(graph_.GetAllConnections().size(), 1U);
}

TEST_F(PropertyChangeTest, ListsEveryLinkedNode) {
    graph_.Connect(literal_, literal_out_, first_, first_in_);
    graph_.Connect(literal_, literal_out_, second_, second_in_);

    const auto kOutcome = Apply(literal_, ToFloat(), &nodes_);

    EXPECT_FALSE(kOutcome.applied_);
    EXPECT_EQ(kOutcome.blocking_connections_.size(), 2U);
    EXPECT_NE(kOutcome.error_.find("« Operator », « Print »"),
              std::string::npos);
    EXPECT_NE(kOutcome.error_.find("Supprimez d'abord ces liaisons."),
              std::string::npos);
}

TEST_F(PropertyChangeTest, AppliesOnceTheLinkIsRemoved) {
    const core::ConnectionId kLink =
        graph_.Connect(literal_, literal_out_, first_, first_in_);
    ASSERT_FALSE(Apply(literal_, ToFloat(), &nodes_).applied_);

    graph_.Disconnect(kLink);

    EXPECT_TRUE(Apply(literal_, ToFloat(), &nodes_).applied_);
    EXPECT_EQ(graph_.GetNode(literal_)->OutputPin(literal_out_)->type_, kFloat);
}

TEST_F(PropertyChangeTest, ALinkOnAnotherPinDoesNotBlock) {
    // The input pin keeps its type: its link is not affected.
    const core::NodeId kSource = graph_.CreateNode(kPlainNode).Id();
    const core::PinId kSourceOut = graph_.AddOutputPin(kSource, "out", kInt);
    graph_.Connect(kSource, kSourceOut, literal_, 1);

    EXPECT_TRUE(Apply(literal_, ToFloat(), &nodes_).applied_);
}

TEST_F(PropertyChangeTest, WithoutNodeListOnlyPropertiesChange) {
    const auto kOutcome = Apply(literal_, ToFloat(), nullptr);

    EXPECT_TRUE(kOutcome.applied_);
    EXPECT_TRUE(kOutcome.pin_changes_.empty());
    EXPECT_EQ(graph_.GetNode(literal_)->OutputPin(literal_out_)->type_, kInt);
}

TEST_F(PropertyChangeTest, UnknownNodeTypeKeepsItsPins) {
    const core::NodeId kBroken = graph_.CreateNode(kBrokenNode).Id();

    const auto kOutcome = Apply(
        kBroken, {{kValueProperty, {.type_id_ = 0, .value_ = 1.0}}}, &nodes_);

    EXPECT_TRUE(kOutcome.applied_);
    EXPECT_TRUE(graph_.GetNode(kBroken)->HasProperty(kValueProperty));
}

TEST_F(PropertyChangeTest, MissingNodeIsReported) {
    const auto kOutcome =
        Apply(999, {{kValueProperty, {.type_id_ = 0, .value_ = 1.0}}}, &nodes_);

    EXPECT_FALSE(kOutcome.applied_);
    EXPECT_EQ(kOutcome.error_, "Ce nœud n'existe plus.");
}

}  // namespace
