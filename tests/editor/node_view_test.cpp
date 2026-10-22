/**
 * @file node_view_test.cpp
 * @brief Unit tests for the NodeView class, which represents a visual node in the editor's UI.
 *
 * @author Created by Nolan Papa
 * @date Created on 26-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 11-10-2026
 */

 #include <gtest/gtest.h>

#include "graph/graph.hpp"
#include "ui/node_view.hpp"
#include "render/camera.hpp"

TEST(NodeViewHitTest, UsesWorldCoordinatesForBodyAndPorts) {
    core::Graph graph;
    core::Node &node = graph.CreateNode(7);
    const core::PinId input = graph.AddInputPin(node.Id(), "input", 1);
    const core::PinId output = graph.AddOutputPin(node.Id(), "output", 2);
    editor::ui::NodeView view(node, "Example", {100.0F, 80.0F});

    EXPECT_EQ(view.HitTest({120.0F, 100.0F}).part_,
              editor::ui::HitPart::kBody);
    EXPECT_EQ(view.HitTest({100.0F, 118.0F}).pin_id_, input);
    EXPECT_EQ(view.HitTest({view.Bounds().x_ + view.Bounds().width_, 118.0F})
                  .pin_id_,
              output);
    EXPECT_EQ(view.HitTest({20.0F, 20.0F}).part_,
              editor::ui::HitPart::kNone);
}

TEST(NodeViewHitTest, CameraRoundTripPreservesWorldPoint) {
    render::Camera camera;
    camera.SetViewport(800.0F, 600.0F);
    camera.ZoomAt(utils::WrappedVector2{400.0F, 300.0F}, 3.0F);
    camera.Pan(utils::WrappedVector2{30.0F, 20.0F});

    const auto kScreen = camera.WorldToScreen(utils::WrappedVector2{12.0F, 9.0F});
    const auto kWorld = camera.ScreenToWorld(kScreen);

    EXPECT_FLOAT_EQ(kWorld.x_, 12.00001F);
    EXPECT_FLOAT_EQ(kWorld.y_, 9.0F);
}

TEST(NodeViewLayout, KeepsLongTitlesWithinBoundedWidth) {
    core::Graph graph;
    core::Node &node = graph.CreateNode(3);
    graph.AddInputPin(node.Id(), "a very long input port name", 1);
    editor::ui::NodeView view(node, "a very long node title", {0.0F, 0.0F});

    EXPECT_GE(view.Bounds().width_, 150.0F);
    EXPECT_LE(view.Bounds().width_, 360.0F);
}

TEST(NodeViewContent, GrowsToFitTheFieldsBelowThePins) {
    core::Graph graph;
    core::Node &node = graph.CreateNode(2);
    graph.AddOutputPin(node.Id(), "out", 1);
    editor::ui::NodeView view(node, "Literal", {10.0F, 20.0F});
    const utils::WrappedRectangle kBefore = view.Bounds();
    EXPECT_EQ(view.ContentBounds().height_, 0.0F);

    view.SetContentSize({250.0F, 48.0F});

    const utils::WrappedRectangle kContent = view.ContentBounds();
    EXPECT_GE(view.Bounds().width_, 250.0F);
    EXPECT_GT(view.Bounds().height_, kBefore.height_ + 48.0F - 1.0F);
    EXPECT_FLOAT_EQ(kContent.height_, 48.0F);
    EXPECT_GE(kContent.y_, kBefore.y_ + 30.0F + 24.0F);  // header + pin row
    EXPECT_LE(kContent.y_ + kContent.height_,
              view.Bounds().y_ + view.Bounds().height_);
    // The output pin follows the right edge.
    const auto kPin = view.GetPinPosition(1, false);
    ASSERT_TRUE(kPin.has_value());
    EXPECT_FLOAT_EQ(kPin->x_, view.Bounds().x_ + view.Bounds().width_);

    view.SetContentSize({0.0F, 0.0F});
    EXPECT_FLOAT_EQ(view.Bounds().height_, kBefore.height_);
}

TEST(NodeViewContent, RefreshKeepsThePositionAndTheContent) {
    core::Graph graph;
    core::Node &node = graph.CreateNode(2);
    const core::PinId out = graph.AddOutputPin(node.Id(), "out", 1);
    editor::ui::NodeView view(node, "Literal", {10.0F, 20.0F});
    view.SetContentSize({100.0F, 24.0F});
    view.MoveBy({5.0F, 5.0F});
    const utils::WrappedRectangle kBefore = view.Bounds();

    graph.SetPinType(node.Id(), out, false, 3);
    view.Refresh(*graph.GetNode(node.Id()));

    EXPECT_FLOAT_EQ(view.Bounds().x_, kBefore.x_);
    EXPECT_FLOAT_EQ(view.Bounds().y_, kBefore.y_);
    EXPECT_FLOAT_EQ(view.Bounds().height_, kBefore.height_);
}
