/**
 * @file node_canvas_test.cpp
 * @brief Tests for the NodeCanvas selection and removal helpers, and for the
 * pin positions links are drawn between
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 */

#include "ui/node_canvas.hpp"

#include <gtest/gtest.h>

#include <vector>

#include "graph/graph.hpp"

namespace {

struct Canvas {
    core::Graph graph_;
    editor::ui::NodeCanvas canvas_;

    core::NodeId Add(float x) {
        core::Node &node = graph_.CreateNode(1);
        graph_.AddInputPin(node.Id(), "in", 1);
        graph_.AddOutputPin(node.Id(), "out", 1);
        canvas_.AddNode(node, "Node", {x, 0.0F});
        return node.Id();
    }
};

}  // namespace

TEST(NodeCanvasSelection, NothingSelectedByDefault) {
    Canvas canvas;
    canvas.Add(0.0F);
    canvas.Add(200.0F);

    EXPECT_TRUE(canvas.canvas_.SelectedNodes().empty());
}

TEST(NodeCanvasSelection, SetSelectionSelectsExactlyTheGivenNodes) {
    Canvas canvas;
    const core::NodeId kA = canvas.Add(0.0F);
    const core::NodeId kB = canvas.Add(200.0F);
    const core::NodeId kC = canvas.Add(400.0F);

    canvas.canvas_.SetSelection({kA, kC});
    EXPECT_EQ(canvas.canvas_.SelectedNodes(),
              (std::vector<core::NodeId>{kA, kC}));

    canvas.canvas_.SetSelection({kB, 999});
    EXPECT_EQ(canvas.canvas_.SelectedNodes(), (std::vector<core::NodeId>{kB}));
    EXPECT_FALSE(canvas.canvas_.Views().at(kA)->Selected());
}

TEST(NodeCanvasSelection, SetSelectionBringsNodesToFront) {
    Canvas canvas;
    const core::NodeId kA = canvas.Add(0.0F);
    const core::NodeId kB = canvas.Add(0.0F);  // on top of kA

    ASSERT_EQ(canvas.canvas_.HitTest({20.0F, 15.0F})->node_id_, kB);
    canvas.canvas_.SetSelection({kA});
    EXPECT_EQ(canvas.canvas_.HitTest({20.0F, 15.0F})->node_id_, kA);
}

TEST(NodeCanvasRemoval, RemovesViewsOfDeletedNodes) {
    Canvas canvas;
    const core::NodeId kA = canvas.Add(0.0F);
    const core::NodeId kB = canvas.Add(200.0F);
    const core::NodeId kC = canvas.Add(400.0F);
    canvas.canvas_.SetSelection({kA, kB});

    canvas.graph_.RemoveNode(kA);
    canvas.graph_.RemoveNode(kB);
    canvas.canvas_.RemoveMissingNodes(canvas.graph_);

    EXPECT_EQ(canvas.canvas_.Views().size(), 1U);
    EXPECT_TRUE(canvas.canvas_.Views().contains(kC));
    EXPECT_TRUE(canvas.canvas_.SelectedNodes().empty());
    EXPECT_FALSE(canvas.canvas_.HitTest({20.0F, 15.0F}).has_value());
}

TEST(NodeCanvasLinks, InputAndOutputWithTheSameIdHaveDistinctPositions) {
    Canvas canvas;
    const core::NodeId kA = canvas.Add(0.0F);
    const auto &view = *canvas.canvas_.Views().at(kA);
    const core::PinId kPin = 1;  // first input and first output

    const auto kInput = view.GetPinPosition(kPin, true);
    const auto kOutput = view.GetPinPosition(kPin, false);
    ASSERT_TRUE(kInput && kOutput);

    // Inputs sit on the left edge, outputs on the right edge.
    EXPECT_FLOAT_EQ(kInput->x_, view.Bounds().x_);
    EXPECT_FLOAT_EQ(kOutput->x_, view.Bounds().x_ + view.Bounds().width_);
}

TEST(NodeCanvasSelection, NoSelectionRectangleWithoutDrag) {
    Canvas canvas;
    canvas.Add(0.0F);

    EXPECT_FALSE(canvas.canvas_.ActiveSelectionRectangle().has_value());
}
