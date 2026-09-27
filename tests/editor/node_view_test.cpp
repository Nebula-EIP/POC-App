/**
 * @file node_view_test.cpp
 * @brief Unit tests for the NodeView class, which represents a visual node in the editor's UI.
 *
 * @author Created by Nolan Papa
 * @date Created on 26-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 26-09-2026
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

    EXPECT_FLOAT_EQ(kWorld.x_, 12.0F);
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