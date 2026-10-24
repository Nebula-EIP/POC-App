/**
 * @file node_canvas_test.cpp
 * @brief Tests for the NodeCanvas selection and removal helpers, for the
 * pin positions links are drawn between, and for the end of a link drag (on
 * a pin, on a node body, on empty canvas)
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 24-10-2026
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

// ---------------------------------------------------------------------------
// End of a link drag

namespace {

using editor::ui::CanvasInput;
using editor::ui::HitPart;

CanvasInput At(utils::WrappedVector2 cursor, bool pressed, bool down) {
    CanvasInput input;
    input.cursor_ = cursor;
    input.left_pressed_ = pressed;
    input.left_down_ = down;
    return input;
}

/// Presses on the output pin of a node, then drags to a point.
void DragFromOutput(Canvas &canvas, core::NodeId node,
                    utils::WrappedVector2 to) {
    const auto kPin = canvas.canvas_.Views().at(node)->GetPinPosition(1, false);
    ASSERT_TRUE(kPin.has_value());
    canvas.canvas_.Update(At(*kPin, true, true));
    canvas.canvas_.Update(At(to, false, true));
}

}  // namespace

TEST(NodeCanvasLinkDrop, ReleaseOnEmptyCanvasRecordsALinkDrop) {
    Canvas canvas;
    const core::NodeId kA = canvas.Add(0.0F);
    const utils::WrappedVector2 kEmpty{600.0F, 300.0F};

    DragFromOutput(canvas, kA, kEmpty);
    ASSERT_TRUE(canvas.canvas_.GetConnectionDragLine().has_value());
    canvas.canvas_.Update(At(kEmpty, false, false));

    const auto kDrop = canvas.canvas_.PopPendingLinkDrop();
    ASSERT_TRUE(kDrop.has_value());
    EXPECT_EQ(kDrop->pin_.node_id_, kA);
    EXPECT_EQ(kDrop->pin_.pin_id_, 1U);
    EXPECT_EQ(kDrop->pin_.part_, HitPart::kOutputPin);
    EXPECT_FLOAT_EQ(kDrop->world_.x_, kEmpty.x_);
    EXPECT_FLOAT_EQ(kDrop->world_.y_, kEmpty.y_);
    EXPECT_FALSE(canvas.canvas_.PopPendingConnectionRequest().has_value());
    // The drag itself is over; popped once.
    EXPECT_FALSE(canvas.canvas_.GetConnectionDragLine().has_value());
    EXPECT_FALSE(canvas.canvas_.PopPendingLinkDrop().has_value());
}

TEST(NodeCanvasLinkDrop, ReleaseOnAPinRequestsAConnectionInstead) {
    Canvas canvas;
    const core::NodeId kA = canvas.Add(0.0F);
    const core::NodeId kB = canvas.Add(300.0F);
    const auto kInput = canvas.canvas_.Views().at(kB)->GetPinPosition(1, true);
    ASSERT_TRUE(kInput.has_value());

    DragFromOutput(canvas, kA, *kInput);
    canvas.canvas_.Update(At(*kInput, false, false));

    const auto kRequest = canvas.canvas_.PopPendingConnectionRequest();
    ASSERT_TRUE(kRequest.has_value());
    EXPECT_EQ(kRequest->first.node_id_, kA);
    EXPECT_EQ(kRequest->second.node_id_, kB);
    EXPECT_EQ(kRequest->second.part_, HitPart::kInputPin);
    EXPECT_FALSE(canvas.canvas_.PopPendingLinkDrop().has_value());
}

TEST(NodeCanvasLinkDrop, ReleaseOnANodeBodyDoesNothing) {
    Canvas canvas;
    const core::NodeId kA = canvas.Add(0.0F);
    const core::NodeId kB = canvas.Add(300.0F);
    const auto &bounds = canvas.canvas_.Views().at(kB)->Bounds();
    const utils::WrappedVector2 kBody{bounds.x_ + bounds.width_ / 2.0F,
                                      bounds.y_ + bounds.height_ / 2.0F};
    ASSERT_EQ(canvas.canvas_.HitTest(kBody)->part_, HitPart::kBody);

    DragFromOutput(canvas, kA, kBody);
    canvas.canvas_.Update(At(kBody, false, false));

    EXPECT_FALSE(canvas.canvas_.PopPendingLinkDrop().has_value());
    EXPECT_FALSE(canvas.canvas_.PopPendingConnectionRequest().has_value());
}

TEST(NodeCanvasLinkDrop, ReleaseOutsideTheWindowDoesNothing) {
    Canvas canvas;
    const core::NodeId kA = canvas.Add(0.0F);
    const utils::WrappedVector2 kEmpty{600.0F, 300.0F};

    DragFromOutput(canvas, kA, kEmpty);
    CanvasInput release = At(kEmpty, false, false);
    release.in_window_ = false;
    canvas.canvas_.Update(release);

    EXPECT_FALSE(canvas.canvas_.PopPendingLinkDrop().has_value());
    EXPECT_FALSE(canvas.canvas_.GetConnectionDragLine().has_value());
}

TEST(NodeCanvasLinkDrop, ReleaseOverACapturingOverlayEndsTheDrag) {
    // Released on the top bar: the bar owns the pointer, the drag ends there
    // and is not picked up later on the canvas.
    Canvas canvas;
    const core::NodeId kA = canvas.Add(0.0F);
    const utils::WrappedVector2 kEmpty{600.0F, 300.0F};

    DragFromOutput(canvas, kA, kEmpty);
    CanvasInput release = At(kEmpty, false, false);
    release.pointer_captured_ = true;
    canvas.canvas_.Update(release);
    canvas.canvas_.Update(At(kEmpty, false, false));

    EXPECT_FALSE(canvas.canvas_.PopPendingLinkDrop().has_value());
    EXPECT_FALSE(canvas.canvas_.GetConnectionDragLine().has_value());
}

TEST(NodeCanvasLinkDrop, ReleaseKeepsTheSelection) {
    // The release that opens the picker is not a click on the canvas.
    Canvas canvas;
    const core::NodeId kA = canvas.Add(0.0F);
    const core::NodeId kB = canvas.Add(300.0F);
    canvas.canvas_.SetSelection({kB});
    const utils::WrappedVector2 kEmpty{600.0F, 300.0F};

    DragFromOutput(canvas, kA, kEmpty);
    canvas.canvas_.Update(At(kEmpty, false, false));

    EXPECT_TRUE(canvas.canvas_.PopPendingLinkDrop().has_value());
    EXPECT_EQ(canvas.canvas_.SelectedNodes(), (std::vector<core::NodeId>{kB}));
    EXPECT_FALSE(canvas.canvas_.ActiveSelectionRectangle().has_value());
}

TEST(NodeCanvasLinkDrop, LinkDropOfARemovedNodeIsForgotten) {
    Canvas canvas;
    const core::NodeId kA = canvas.Add(0.0F);
    const utils::WrappedVector2 kEmpty{600.0F, 300.0F};
    DragFromOutput(canvas, kA, kEmpty);
    canvas.canvas_.Update(At(kEmpty, false, false));

    canvas.graph_.RemoveNode(kA);
    canvas.canvas_.RemoveMissingNodes(canvas.graph_);

    EXPECT_FALSE(canvas.canvas_.PopPendingLinkDrop().has_value());
}
