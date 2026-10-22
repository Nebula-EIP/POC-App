/**
 * @file node_clipboard_test.cpp
 * @brief Tests for the node clipboard (Ctrl+C / Ctrl+V)
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 12-10-2026
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

// AddNode() gives every node one input and one output pin, both with id 1.
constexpr core::PinId kIn = 1;
constexpr core::PinId kOut = 1;

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

// ---------------------------------------------------------------------------
// Connections

TEST(NodeClipboard, PasteLinksTheCopiesLikeTheOriginals) {
    core::Graph graph;
    const core::NodeId kA = AddNode(graph, 1);
    const core::NodeId kB = AddNode(graph, 2);
    const core::NodeId kC = AddNode(graph, 3);
    graph.Connect(kA, kOut, kB, kIn);  // A -> B
    graph.Connect(kB, kOut, kC, kIn);  // B -> C
    NodeClipboard clipboard;
    clipboard.Copy(graph, {{kA, "A", {0.0F, 0.0F}},
                           {kB, "B", {100.0F, 0.0F}},
                           {kC, "C", {200.0F, 0.0F}}});

    const auto kPasted = clipboard.Paste(graph, {0.0F, 300.0F});

    ASSERT_EQ(kPasted.size(), 3U);
    EXPECT_NE(graph.GetConnectionId(kPasted[0].id_, kOut, kPasted[1].id_, kIn),
              0U);
    EXPECT_NE(graph.GetConnectionId(kPasted[1].id_, kOut, kPasted[2].id_, kIn),
              0U);
    // The originals keep their links, and nothing links copies to originals.
    EXPECT_EQ(graph.GetAllConnections().size(), 4U);
    EXPECT_NE(graph.GetConnectionId(kA, kOut, kB, kIn), 0U);
    EXPECT_NE(graph.GetConnectionId(kB, kOut, kC, kIn), 0U);
}

TEST(NodeClipboard, LinksToNodesThatWereNotCopiedAreNotPasted) {
    core::Graph graph;
    const core::NodeId kOutside = AddNode(graph, 1);
    const core::NodeId kA = AddNode(graph, 2);
    const core::NodeId kB = AddNode(graph, 3);
    graph.Connect(kOutside, kOut, kA, kIn);  // from outside the copy
    graph.Connect(kA, kOut, kB, kIn);        // inside the copy
    graph.Connect(kB, kOut, kOutside, kIn);  // to outside the copy
    NodeClipboard clipboard;
    clipboard.Copy(graph, {{kA, "A", {0.0F, 0.0F}}, {kB, "B", {0.0F, 0.0F}}});

    const auto kPasted = clipboard.Paste(graph, {0.0F, 0.0F});

    ASSERT_EQ(kPasted.size(), 2U);
    EXPECT_EQ(graph.GetAllConnections().size(), 4U);  // 3 originals + 1 copy
    EXPECT_NE(graph.GetConnectionId(kPasted[0].id_, kOut, kPasted[1].id_, kIn),
              0U);
    EXPECT_EQ(graph.GetConnectionId(kOutside, kOut, kPasted[0].id_, kIn), 0U);
    EXPECT_EQ(graph.GetConnectionId(kPasted[1].id_, kOut, kOutside, kIn), 0U);
}

TEST(NodeClipboard, LinksArePastedAfterTheOriginalsAreDeleted) {
    // What Cut (Ctrl+X) then Paste does.
    core::Graph graph;
    const core::NodeId kA = AddNode(graph, 1);
    const core::NodeId kB = AddNode(graph, 2);
    graph.Connect(kA, kOut, kB, kIn);
    NodeClipboard clipboard;
    clipboard.Copy(graph, {{kA, "A", {0.0F, 0.0F}}, {kB, "B", {0.0F, 0.0F}}});
    graph.RemoveNode(kA);
    graph.RemoveNode(kB);
    ASSERT_TRUE(graph.GetAllConnections().empty());

    const auto kPasted = clipboard.Paste(graph, {0.0F, 0.0F});

    ASSERT_EQ(kPasted.size(), 2U);
    ASSERT_EQ(graph.GetAllConnections().size(), 1U);
    EXPECT_NE(graph.GetConnectionId(kPasted[0].id_, kOut, kPasted[1].id_, kIn),
              0U);
}

TEST(NodeClipboard, EachPasteLinksItsOwnCopies) {
    core::Graph graph;
    const core::NodeId kA = AddNode(graph, 1);
    const core::NodeId kB = AddNode(graph, 2);
    graph.Connect(kA, kOut, kB, kIn);
    NodeClipboard clipboard;
    clipboard.Copy(graph, {{kA, "A", {0.0F, 0.0F}}, {kB, "B", {0.0F, 0.0F}}});

    const auto kFirst = clipboard.Paste(graph, {0.0F, 0.0F});
    const auto kSecond = clipboard.Paste(graph, {0.0F, 0.0F});

    EXPECT_EQ(graph.GetAllConnections().size(), 3U);
    EXPECT_NE(graph.GetConnectionId(kFirst[0].id_, kOut, kFirst[1].id_, kIn),
              0U);
    EXPECT_NE(graph.GetConnectionId(kSecond[0].id_, kOut, kSecond[1].id_, kIn),
              0U);
    EXPECT_EQ(graph.GetConnectionId(kFirst[0].id_, kOut, kSecond[1].id_, kIn),
              0U);
}

TEST(NodeClipboard, CopyingANodeOnlyLinkedToItselfKeepsTheLoop) {
    core::Graph graph;
    const core::NodeId kA = AddNode(graph, 1);
    graph.Connect(kA, kOut, kA, kIn);
    NodeClipboard clipboard;
    clipboard.Copy(graph, {{kA, "A", {0.0F, 0.0F}}});

    const auto kPasted = clipboard.Paste(graph, {0.0F, 0.0F});

    ASSERT_EQ(kPasted.size(), 1U);
    EXPECT_NE(graph.GetConnectionId(kPasted[0].id_, kOut, kPasted[0].id_, kIn),
              0U);
}
