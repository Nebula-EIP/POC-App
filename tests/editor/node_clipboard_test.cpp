/**
 * @file node_clipboard_test.cpp
 * @brief Tests for the node clipboard (Ctrl+C / Ctrl+V)
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 */

#include "ui/node_clipboard.hpp"

#include <gtest/gtest.h>

#include <any>
#include <string>
#include <vector>

#include "graph/graph.hpp"

namespace {

using editor::ui::NodeClipboard;
using editor::ui::PlacedNode;

constexpr core::DataType kIntType = 1;

core::NodeId AddNode(core::Graph &graph, core::NodeType type) {
    core::Node &node = graph.CreateNode(type);
    graph.AddInputPin(node.Id(), "in", kIntType);
    graph.AddOutputPin(node.Id(), "out", kIntType);
    return node.Id();
}

}  // namespace

TEST(NodeClipboard, EmptyByDefaultAndPastesNothing) {
    core::Graph graph;
    const NodeClipboard kClipboard;

    EXPECT_TRUE(kClipboard.Empty());
    EXPECT_TRUE(kClipboard.Paste(graph, {0.0F, 0.0F}).empty());
    EXPECT_TRUE(graph.GetAllNodes().empty());
}

TEST(NodeClipboard, PastedNodesKeepTypePinsPropertiesAndTitle) {
    core::Graph graph;
    const core::NodeId kSource = AddNode(graph, 7);
    const core::PropertyId kProperty = graph.GetNode(kSource)->AddProperty(
        core::Property{.type_id_ = 2, .value_ = 42});
    NodeClipboard clipboard;
    clipboard.Copy(graph, {{kSource, "Literal", {10.0F, 20.0F}}});

    const std::vector<PlacedNode> kPasted =
        clipboard.Paste(graph, {100.0F, 100.0F});

    ASSERT_EQ(kPasted.size(), 1U);
    EXPECT_NE(kPasted[0].id_, kSource);
    EXPECT_EQ(kPasted[0].title_, "Literal");
    const core::Node *copy = graph.GetNode(kPasted[0].id_);
    ASSERT_NE(copy, nullptr);
    EXPECT_EQ(copy->Type(), 7);
    EXPECT_EQ(copy->InputPinsCount(), 1U);
    EXPECT_EQ(copy->OutputPinsCount(), 1U);
    ASSERT_TRUE(copy->HasProperty(kProperty));
    EXPECT_EQ(std::any_cast<int>(copy->GetProperty(kProperty)->value_), 42);
}

TEST(NodeClipboard, PasteKeepsTheLayoutRelativeToTheAnchor) {
    core::Graph graph;
    const core::NodeId kA = AddNode(graph, 1);
    const core::NodeId kB = AddNode(graph, 1);
    NodeClipboard clipboard;
    clipboard.Copy(graph,
                   {{kA, "A", {50.0F, 80.0F}}, {kB, "B", {250.0F, 30.0F}}});

    const auto kPasted = clipboard.Paste(graph, {1000.0F, 500.0F});

    ASSERT_EQ(kPasted.size(), 2U);
    // Top-left corner of the group (50, 30) lands on the anchor.
    EXPECT_FLOAT_EQ(kPasted[0].position_.x_, 1000.0F);
    EXPECT_FLOAT_EQ(kPasted[0].position_.y_, 550.0F);
    EXPECT_FLOAT_EQ(kPasted[1].position_.x_, 1200.0F);
    EXPECT_FLOAT_EQ(kPasted[1].position_.y_, 500.0F);
}

TEST(NodeClipboard, PastedNodesHaveNoConnection) {
    core::Graph graph;
    const core::NodeId kA = AddNode(graph, 1);
    const core::NodeId kB = AddNode(graph, 1);
    graph.Connect(kA, 1, kB, 1);
    NodeClipboard clipboard;
    clipboard.Copy(graph, {{kA, "A", {0.0F, 0.0F}}, {kB, "B", {200.0F, 0.0F}}});

    clipboard.Paste(graph, {0.0F, 300.0F});

    EXPECT_EQ(graph.GetAllNodes().size(), 4U);
    EXPECT_EQ(graph.GetAllConnections().size(), 1U);
}

TEST(NodeClipboard, CanPasteSeveralTimes) {
    core::Graph graph;
    const core::NodeId kA = AddNode(graph, 1);
    NodeClipboard clipboard;
    clipboard.Copy(graph, {{kA, "A", {0.0F, 0.0F}}});

    const auto kFirst = clipboard.Paste(graph, {0.0F, 100.0F});
    const auto kSecond = clipboard.Paste(graph, {0.0F, 200.0F});

    ASSERT_EQ(kFirst.size(), 1U);
    ASSERT_EQ(kSecond.size(), 1U);
    EXPECT_NE(kFirst[0].id_, kSecond[0].id_);
    EXPECT_EQ(graph.GetAllNodes().size(), 3U);
}

TEST(NodeClipboard, StillPastesAfterTheOriginalIsDeleted) {
    core::Graph graph;
    const core::NodeId kA = AddNode(graph, 5);
    NodeClipboard clipboard;
    clipboard.Copy(graph, {{kA, "A", {0.0F, 0.0F}}});
    graph.RemoveNode(kA);

    const auto kPasted = clipboard.Paste(graph, {0.0F, 0.0F});

    ASSERT_EQ(kPasted.size(), 1U);
    EXPECT_EQ(graph.GetNode(kPasted[0].id_)->Type(), 5);
}

TEST(NodeClipboard, CopyingNothingKeepsThePreviousContent) {
    core::Graph graph;
    const core::NodeId kA = AddNode(graph, 1);
    NodeClipboard clipboard;
    clipboard.Copy(graph, {{kA, "A", {0.0F, 0.0F}}});

    clipboard.Copy(graph, {});
    clipboard.Copy(graph, {{999, "missing", {0.0F, 0.0F}}});

    EXPECT_EQ(clipboard.Size(), 1U);
}

TEST(NodeClipboard, NewCopyReplacesThePreviousContent) {
    core::Graph graph;
    const core::NodeId kA = AddNode(graph, 1);
    const core::NodeId kB = AddNode(graph, 2);
    const core::NodeId kC = AddNode(graph, 3);
    NodeClipboard clipboard;
    clipboard.Copy(graph, {{kA, "A", {0.0F, 0.0F}}});

    clipboard.Copy(graph, {{kB, "B", {0.0F, 0.0F}}, {kC, "C", {0.0F, 0.0F}}});
    const auto kPasted = clipboard.Paste(graph, {0.0F, 0.0F});

    ASSERT_EQ(kPasted.size(), 2U);
    EXPECT_EQ(kPasted[0].title_, "B");
    EXPECT_EQ(kPasted[1].title_, "C");
}
