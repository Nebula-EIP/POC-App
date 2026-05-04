#include <gtest/gtest.h>

#include <filesystem>

#include "graph.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/operator_node.hpp"
#include "utils/logger.hpp"

class GraphBasicTest : public testing::Test {
 protected:
    core::Graph graph_;
};

TEST_F(GraphBasicTest, CreateLiteralNode) {
    auto *node = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetDisplayName(), node->name());
    EXPECT_EQ(node->GetCategory(), "Cool stuff~");
    EXPECT_EQ(node->GetOutputPinCount(), 1);
}

TEST_F(GraphBasicTest, CreateOperatorNode) {
    auto *node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetDisplayName(), node->name());
    EXPECT_EQ(node->GetCategory(), std::string("Arithmetic"));
    EXPECT_EQ(node->GetInputPinCount(), 2);
    EXPECT_EQ(node->GetOutputPinCount(), 1);
}

TEST_F(GraphBasicTest, LinkAndUnlinkCreatesAndRemovesConnection) {
    auto *lit = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto *op = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator);
    ASSERT_NE(lit, nullptr);
    ASSERT_NE(op, nullptr);

    // Ensure types match (literal default is int, operator addition expects int)
    EXPECT_NO_THROW(graph_.Link(lit, 0, op, 0));

    // After linking, parent should report connected
    EXPECT_TRUE(op->IsInputPinConnected(0));

    // Unlink
    EXPECT_NO_THROW(graph_.Unlink(lit, 0, op, 0));
    EXPECT_FALSE(op->IsInputPinConnected(0));
}

TEST_F(GraphBasicTest, SerializeDeserializeRoundTrip) {
    core::Graph g;
    auto *l1 = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto *l2 = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto *op = g.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator);

    l1->set_name("L1");
    l2->set_name("L2");

    // link l1 -> op.A, l2 -> op.B
    g.Link(l1, 0, op, 0);
    g.Link(l2, 0, op, 1);

    // serialize to json
    auto json = g.Serialize();
    EXPECT_TRUE(json.contains("graph"));
    EXPECT_TRUE(json["graph"].contains("nodes"));
    EXPECT_TRUE(json["graph"].contains("connections"));

    // save to temp file and reload
    std::filesystem::path p = "/tmp/graph_basic_test_graph.nebula";
    auto res = g.SaveToFile(p);
    EXPECT_TRUE(res.has_value());
    auto loaded = core::Graph::LoadFromFile(p);
    EXPECT_TRUE(loaded.has_value());
    auto g2 = std::move(loaded).value();
    EXPECT_EQ(g2.GetAllNodes().size(), 3);
    EXPECT_EQ(g2.Serialize()["graph"]["connections"].size(), 2);

    std::filesystem::remove(p);
}
