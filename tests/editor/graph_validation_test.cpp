/**
 * @file graph_validation_test.cpp
 * @brief Unit tests for editor graph validation highlighting and workflow.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 21-11-2026
 */

#include "ui/graph_validation.hpp"

#include <gtest/gtest.h>

#include <string>

#include "graph/graph.hpp"
#include "nodes/c_node_list_capability.hpp"
#include "ui/node_canvas.hpp"
#include "validation/c_validation_capability.hpp"

TEST(EditorGraphValidationTest,
     HighlightsAffectedNodesAndPinsAndAllowsFixingIncompleteGraph) {
    core::Graph graph;
    editor::ui::NodeCanvas canvas;
    const c_module::CValidationCapability kValidator;

    // Create an incomplete Operator node (missing right input) and a Literal
    core::Node &lit1 = graph.CreateNode(2);
    graph.AddOutputPin(lit1.Id(), "out", 2);
    lit1.SetProperty(c_module::properties::kType,
                     {c_module::properties::kType, std::string("int")});
    lit1.SetProperty(c_module::properties::kValue,
                     {c_module::properties::kValue, 5.0});
    canvas.AddNode(lit1, "Literal 1", {0.0F, 0.0F});

    core::Node &op = graph.CreateNode(3);
    const core::PinId kLeftPin = graph.AddInputPin(op.Id(), "left", 2);
    const core::PinId kRightPin = graph.AddInputPin(op.Id(), "right", 2);
    graph.AddOutputPin(op.Id(), "result", 2);
    op.SetProperty(c_module::properties::kOperator,
                   {c_module::properties::kOperator, std::string("+")});
    op.SetProperty(c_module::properties::kType,
                   {c_module::properties::kType, std::string("int")});
    canvas.AddNode(op, "Addition", {200.0F, 0.0F});

    // Connect only left pin -> graph is still incomplete while user is editing
    graph.Connect(lit1.Id(), 1, op.Id(), kLeftPin);

    // 1. Validate before export/compilation: should fail and highlight Addition
    const auto kInvalidResult =
        editor::ui::ValidateGraphForCodegen(graph, canvas, kValidator);
    EXPECT_FALSE(kInvalidResult.is_valid_);
    ASSERT_EQ(kInvalidResult.ErrorCount(), 1U);
    EXPECT_NE(kInvalidResult.errors_[0].message_.find("Operator"),
              std::string::npos);
    EXPECT_NE(kInvalidResult.errors_[0].message_.find("right"),
              std::string::npos);

    const auto &views = canvas.Views();
    EXPECT_FALSE(views.at(lit1.Id())->HasError());
    EXPECT_TRUE(views.at(op.Id())->HasError());
    EXPECT_EQ(views.at(op.Id())->ErrorPinId(), kRightPin);
    EXPECT_TRUE(views.at(op.Id())->ErrorPinIsInput());

    // 2. User continues editing the incomplete graph and connects the second
    // input
    core::Node &lit2 = graph.CreateNode(2);
    graph.AddOutputPin(lit2.Id(), "out", 2);
    lit2.SetProperty(c_module::properties::kType,
                     {c_module::properties::kType, std::string("int")});
    lit2.SetProperty(c_module::properties::kValue,
                     {c_module::properties::kValue, 10.0});
    canvas.AddNode(lit2, "Literal 2", {0.0F, 100.0F});
    graph.Connect(lit2.Id(), 1, op.Id(), kRightPin);

    // 3. Re-validate: passes and clears error highlights
    const auto kValidResult =
        editor::ui::ValidateGraphForCodegen(graph, canvas, kValidator);
    EXPECT_TRUE(kValidResult.is_valid_);
    EXPECT_EQ(kValidResult.ErrorCount(), 0U);
    EXPECT_FALSE(views.at(op.Id())->HasError());
    EXPECT_EQ(views.at(op.Id())->ErrorPinId(), 0U);
}
