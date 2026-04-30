#include <gtest/gtest.h>

#include "graph.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/operator_node.hpp"

class EdgeStorageTest : public testing::Test {
  protected:
    core::Graph g;
};

TEST_F(EdgeStorageTest, LinkRecordsEdge) {
    auto *l = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto *op = g.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator);
    ASSERT_NE(l, nullptr);
    ASSERT_NE(op, nullptr);

    EXPECT_NO_THROW(g.Link(l, 0, op, 0));
    auto &edges = g.GetEdges();
    ASSERT_EQ(edges.size(), 1u);
    const auto &e = edges[0];
    EXPECT_EQ(e.source_node_id, l->id());
    EXPECT_EQ(e.target_node_id, op->id());
    EXPECT_EQ(e.source_pin, 0u);
    EXPECT_EQ(e.target_pin, 0u);
}

TEST_F(EdgeStorageTest, SerializeIncludesEdgesAndDeserializeRestoresThem) {
    core::Graph g2;
    auto *l1 = g2.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto *l2 = g2.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto *op = g2.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator);

    g2.Link(l1, 0, op, 0);
    g2.Link(l2, 0, op, 1);

    auto json = g2.Serialize();
    ASSERT_TRUE(json.contains("graph"));
    ASSERT_TRUE(json["graph"].contains("edges"));
    EXPECT_EQ(json["graph"]["edges"].size(), 2u);

    auto loaded = core::Graph::Deserialize(json);
    ASSERT_TRUE(loaded.has_value());
    auto g3 = std::move(loaded).value();
    EXPECT_EQ(g3.GetEdges().size(), 2u);
}
