/**
 * @file graph_duplicate_tests.cpp
 * @brief Tests for Graph::DuplicateNode
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 */

#include <gtest/gtest.h>

#include <any>
#include <string>

#include "exception/graph_exception/node_exception.hpp"
#include "graph/graph.hpp"

namespace {

constexpr core::NodeType kNodeType = 4;
constexpr core::DataType kIntType = 1;
constexpr core::DataType kFloatType = 2;

}  // namespace

TEST(GraphDuplicateNode, CopyHasNewIdAndSameType) {
    core::Graph graph;
    const core::NodeId kSource = graph.CreateNode(kNodeType).Id();

    const core::Node &copy = graph.DuplicateNode(kSource);

    EXPECT_NE(copy.Id(), kSource);
    EXPECT_EQ(copy.Type(), kNodeType);
    EXPECT_EQ(graph.GetAllNodes().size(), 2U);
}

TEST(GraphDuplicateNode, CopiesPinsWithTheirIdsNamesAndTypes) {
    core::Graph graph;
    const core::NodeId kSource = graph.CreateNode(kNodeType).Id();
    graph.AddInputPin(kSource, "a", kIntType);
    const core::PinId kRemoved = graph.AddInputPin(kSource, "gone", kIntType);
    graph.AddInputPin(kSource, "b", kFloatType);
    graph.AddOutputPin(kSource, "result", kFloatType);
    graph.RemoveInputPin(kSource, kRemoved);

    const core::Node &copy = graph.DuplicateNode(kSource);
    const core::Node &source = *graph.GetNode(kSource);

    ASSERT_EQ(copy.InputPinsCount(), source.InputPinsCount());
    for (std::size_t i = 0; i < source.InputPinsCount(); ++i) {
        EXPECT_EQ(copy.InputPins()[i].id_, source.InputPins()[i].id_);
        EXPECT_EQ(copy.InputPins()[i].name_, source.InputPins()[i].name_);
        EXPECT_EQ(copy.InputPins()[i].type_, source.InputPins()[i].type_);
    }
    ASSERT_EQ(copy.OutputPinsCount(), 1U);
    EXPECT_EQ(copy.OutputPins()[0].name_, "result");
    EXPECT_EQ(copy.OutputPins()[0].type_, kFloatType);
}

TEST(GraphDuplicateNode, NewPinsOnTheCopyDoNotReuseIds) {
    core::Graph graph;
    const core::NodeId kSource = graph.CreateNode(kNodeType).Id();
    const core::PinId kFirst = graph.AddInputPin(kSource, "a", kIntType);

    const core::NodeId kCopy = graph.DuplicateNode(kSource).Id();
    const core::PinId kAdded = graph.AddInputPin(kCopy, "b", kIntType);

    EXPECT_NE(kAdded, kFirst);
}

TEST(GraphDuplicateNode, CopiesPropertiesAsIndependentValues) {
    core::Graph graph;
    core::Node &source_node = graph.CreateNode(kNodeType);
    const core::NodeId kSource = source_node.Id();
    const core::PropertyId kProperty = source_node.AddProperty(
        core::Property{.type_id_ = 3, .value_ = std::string("hello")});

    core::Node &copy = graph.DuplicateNode(kSource);
    ASSERT_TRUE(copy.HasProperty(kProperty));
    EXPECT_EQ(copy.GetProperty(kProperty)->type_id_, 3U);
    EXPECT_EQ(std::any_cast<std::string>(copy.GetProperty(kProperty)->value_),
              "hello");

    copy.GetProperty(kProperty)->value_ = std::string("changed");
    EXPECT_EQ(std::any_cast<std::string>(
                  graph.GetNode(kSource)->GetProperty(kProperty)->value_),
              "hello");
}

TEST(GraphDuplicateNode, CopyStartsWithoutConnections) {
    core::Graph graph;
    const core::NodeId kUpstream = graph.CreateNode(kNodeType).Id();
    const core::PinId kUpOut = graph.AddOutputPin(kUpstream, "out", kIntType);
    const core::NodeId kSource = graph.CreateNode(kNodeType).Id();
    const core::PinId kIn = graph.AddInputPin(kSource, "in", kIntType);
    const core::PinId kOut = graph.AddOutputPin(kSource, "out", kIntType);
    const core::NodeId kDownstream = graph.CreateNode(kNodeType).Id();
    const core::PinId kDownIn = graph.AddInputPin(kDownstream, "in", kIntType);
    graph.Connect(kUpstream, kUpOut, kSource, kIn);
    graph.Connect(kSource, kOut, kDownstream, kDownIn);

    const core::NodeId kCopy = graph.DuplicateNode(kSource).Id();

    EXPECT_EQ(graph.GetAllConnections().size(), 2U);
    for (const auto &[id, connection] : graph.GetAllConnections()) {
        (void)id;
        EXPECT_NE(connection.from_node_, kCopy);
        EXPECT_NE(connection.to_node_, kCopy);
    }
}

TEST(GraphDuplicateNode, ThrowsOnUnknownNode) {
    core::Graph graph;
    EXPECT_THROW(graph.DuplicateNode(42), core::NodeNotFoundException);
}

TEST(GraphRemoveNode, RemovesTheConnectionsOfTheNode) {
    core::Graph graph;
    const core::NodeId kA = graph.CreateNode(kNodeType).Id();
    const core::PinId kAOut = graph.AddOutputPin(kA, "out", kIntType);
    const core::NodeId kB = graph.CreateNode(kNodeType).Id();
    const core::PinId kBIn = graph.AddInputPin(kB, "in", kIntType);
    const core::PinId kBOut = graph.AddOutputPin(kB, "out", kIntType);
    const core::NodeId kC = graph.CreateNode(kNodeType).Id();
    const core::PinId kCIn = graph.AddInputPin(kC, "in", kIntType);
    graph.Connect(kA, kAOut, kB, kBIn);
    graph.Connect(kB, kBOut, kC, kCIn);
    graph.Connect(kA, kAOut, kC, kCIn);

    ASSERT_TRUE(graph.RemoveNode(kB));

    ASSERT_EQ(graph.GetAllConnections().size(), 1U);
    const core::Connection &kLeft = graph.GetAllConnections().begin()->second;
    EXPECT_EQ(kLeft.from_node_, kA);
    EXPECT_EQ(kLeft.to_node_, kC);
}
