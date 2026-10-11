/**
 * @file connection_request_test.cpp
 * @brief Tests for the canvas connection request (pin drag to connection)
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 */

#include "ui/connection_request.hpp"

#include <gtest/gtest.h>

#include <string>

#include "graph/graph.hpp"

namespace {

using editor::ui::HitPart;
using editor::ui::HitResult;

constexpr core::DataType kIntType = 1;
constexpr core::DataType kFloatType = 2;

struct TwoNodes {
    core::Graph graph_;
    core::NodeId source_ = 0;
    core::NodeId target_ = 0;
    core::PinId source_in_ = 0;
    core::PinId source_out_ = 0;
    core::PinId target_int_in_ = 0;
    core::PinId target_float_in_ = 0;
    core::PinId target_out_ = 0;

    TwoNodes() {
        source_ = graph_.CreateNode(1).Id();
        source_in_ = graph_.AddInputPin(source_, "in", kIntType);
        source_out_ = graph_.AddOutputPin(source_, "out", kIntType);
        target_ = graph_.CreateNode(1).Id();
        target_int_in_ = graph_.AddInputPin(target_, "a", kIntType);
        target_float_in_ = graph_.AddInputPin(target_, "b", kFloatType);
        target_out_ = graph_.AddOutputPin(target_, "out", kIntType);
    }
};

HitResult Input(core::NodeId node, core::PinId pin) {
    return {.node_id_ = node, .pin_id_ = pin, .part_ = HitPart::kInputPin};
}

HitResult Output(core::NodeId node, core::PinId pin) {
    return {.node_id_ = node, .pin_id_ = pin, .part_ = HitPart::kOutputPin};
}

std::string NameTypes(core::DataType type) {
    return type == kIntType ? "int" : type == kFloatType ? "float" : "";
}

}  // namespace

TEST(ConnectionRequest, ConnectsOutputToCompatibleInput) {
    TwoNodes nodes;
    const auto kOutcome = editor::ui::RequestConnection(
        nodes.graph_, Output(nodes.source_, nodes.source_out_),
        Input(nodes.target_, nodes.target_int_in_));

    ASSERT_TRUE(kOutcome.Succeeded());
    EXPECT_TRUE(kOutcome.error_.empty());
    const core::Connection &kLink =
        nodes.graph_.GetAllConnections().at(*kOutcome.connection_);
    EXPECT_EQ(kLink.from_node_, nodes.source_);
    EXPECT_EQ(kLink.out_pin_, nodes.source_out_);
    EXPECT_EQ(kLink.to_node_, nodes.target_);
    EXPECT_EQ(kLink.in_pin_, nodes.target_int_in_);
}

TEST(ConnectionRequest, DragFromInputToOutputConnectsTheSamePins) {
    TwoNodes nodes;
    const auto kOutcome = editor::ui::RequestConnection(
        nodes.graph_, Input(nodes.target_, nodes.target_int_in_),
        Output(nodes.source_, nodes.source_out_));

    ASSERT_TRUE(kOutcome.Succeeded());
    const core::Connection &kLink =
        nodes.graph_.GetAllConnections().at(*kOutcome.connection_);
    EXPECT_EQ(kLink.from_node_, nodes.source_);
    EXPECT_EQ(kLink.out_pin_, nodes.source_out_);
    EXPECT_EQ(kLink.to_node_, nodes.target_);
    EXPECT_EQ(kLink.in_pin_, nodes.target_int_in_);
}

TEST(ConnectionRequest, RejectsTwoInputsWithAMessage) {
    TwoNodes nodes;
    const HitResult kEnd = Input(nodes.target_, nodes.target_int_in_);
    const auto kOutcome = editor::ui::RequestConnection(
        nodes.graph_, Input(nodes.source_, nodes.source_in_), kEnd);

    EXPECT_FALSE(kOutcome.Succeeded());
    EXPECT_NE(kOutcome.error_.find("deux entrées"), std::string::npos);
    ASSERT_TRUE(kOutcome.faulty_pin_.has_value());
    EXPECT_EQ(kOutcome.faulty_pin_->pin_id_, kEnd.pin_id_);
    EXPECT_TRUE(nodes.graph_.GetAllConnections().empty());
}

TEST(ConnectionRequest, RejectsTwoOutputsWithAMessage) {
    TwoNodes nodes;
    const auto kOutcome = editor::ui::RequestConnection(
        nodes.graph_, Output(nodes.source_, nodes.source_out_),
        Output(nodes.target_, nodes.target_out_));

    EXPECT_FALSE(kOutcome.Succeeded());
    EXPECT_NE(kOutcome.error_.find("deux sorties"), std::string::npos);
    EXPECT_TRUE(nodes.graph_.GetAllConnections().empty());
}

TEST(ConnectionRequest, RejectsTypeMismatchNamingBothTypes) {
    TwoNodes nodes;
    const HitResult kEnd = Input(nodes.target_, nodes.target_float_in_);
    const auto kOutcome = editor::ui::RequestConnection(
        nodes.graph_, Output(nodes.source_, nodes.source_out_), kEnd,
        NameTypes);

    EXPECT_FALSE(kOutcome.Succeeded());
    EXPECT_NE(kOutcome.error_.find("incompatibles"), std::string::npos);
    EXPECT_NE(kOutcome.error_.find("« int »"), std::string::npos);
    EXPECT_NE(kOutcome.error_.find("« float »"), std::string::npos);
    ASSERT_TRUE(kOutcome.faulty_pin_.has_value());
    EXPECT_EQ(kOutcome.faulty_pin_->part_, HitPart::kInputPin);
    EXPECT_EQ(kOutcome.faulty_pin_->pin_id_, kEnd.pin_id_);
    EXPECT_TRUE(nodes.graph_.GetAllConnections().empty());
}

TEST(ConnectionRequest, TypeMismatchFallsBackOnTypeIds) {
    TwoNodes nodes;
    const auto kOutcome = editor::ui::RequestConnection(
        nodes.graph_, Output(nodes.source_, nodes.source_out_),
        Input(nodes.target_, nodes.target_float_in_));

    EXPECT_NE(kOutcome.error_.find("« type 1 »"), std::string::npos);
    EXPECT_NE(kOutcome.error_.find("« type 2 »"), std::string::npos);
}

TEST(ConnectionRequest, RejectsLinkingANodeToItself) {
    TwoNodes nodes;
    const auto kOutcome = editor::ui::RequestConnection(
        nodes.graph_, Output(nodes.source_, nodes.source_out_),
        Input(nodes.source_, nodes.source_in_));

    EXPECT_FALSE(kOutcome.Succeeded());
    EXPECT_NE(kOutcome.error_.find("lui-même"), std::string::npos);
    EXPECT_TRUE(nodes.graph_.GetAllConnections().empty());
}

TEST(ConnectionRequest, ReleasingOnTheStartPinDoesNothing) {
    TwoNodes nodes;
    const HitResult kPin = Output(nodes.source_, nodes.source_out_);
    const auto kOutcome =
        editor::ui::RequestConnection(nodes.graph_, kPin, kPin);

    EXPECT_FALSE(kOutcome.Succeeded());
    EXPECT_TRUE(kOutcome.error_.empty());
    EXPECT_FALSE(kOutcome.faulty_pin_.has_value());
}

TEST(ConnectionRequest, InputAndOutputSharingAnIdAreDistinct) {
    // Input and output pins are numbered separately: the first input and the
    // first output of a node have the same id.
    TwoNodes nodes;
    ASSERT_EQ(nodes.source_in_, nodes.source_out_);

    const auto kOutcome = editor::ui::RequestConnection(
        nodes.graph_, Output(nodes.source_, nodes.source_out_),
        Input(nodes.target_, nodes.target_int_in_));

    ASSERT_TRUE(kOutcome.Succeeded());
    EXPECT_EQ(
        nodes.graph_.GetAllConnections().at(*kOutcome.connection_).from_node_,
        nodes.source_);
}
