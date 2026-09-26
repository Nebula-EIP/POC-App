/**
 * @file application.cpp
 * @brief Implementation of the Application class, which manages the main loop,
 * window, and module loading for the editor.
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
    editor::ui::Camera camera;
    camera.offset_ = {30.0F, 20.0F};
    camera.zoom_ = 2.0F;
    const auto screen = camera.WorldToScreen({12.0F, 9.0F});
    const auto world = camera.ScreenToWorld(screen);

    EXPECT_FLOAT_EQ(world.x_, 12.0F);
    EXPECT_FLOAT_EQ(world.y_, 9.0F);
}

TEST(NodeViewLayout, KeepsLongTitlesWithinBoundedWidth) {
    core::Graph graph;
    core::Node &node = graph.CreateNode(3);
    graph.AddInputPin(node.Id(), "a very long input port name", 1);
    editor::ui::NodeView view(node, "a very long node title", {0.0F, 0.0F});

    EXPECT_GE(view.Bounds().width_, 150.0F);
    EXPECT_LE(view.Bounds().width_, 360.0F);
}