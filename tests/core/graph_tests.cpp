#include <gtest/gtest.h>

#include "graph.hpp"
#include "nodes/literal_node.hpp"

class GraphTest : public testing::Test {
 protected:
    core::Graph graph_;
};

// Node Management Tests

TEST_F(GraphTest, AddNode_ValidKind_ReturnsNonNull) {
    EXPECT_NE(graph_.AddNode(core::NodeBase::NodeKind::kLiteral), nullptr);
}

TEST_F(GraphTest, AddNode_UndefinedKind_ReturnsNull) {
    EXPECT_EQ(graph_.AddNode(core::NodeBase::NodeKind::kUndefined), nullptr);
}

TEST_F(GraphTest, AddNode_MultipleNodes_AssignsUniqueIds) {
    auto a = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto b = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto c = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);

    EXPECT_TRUE(
        (a->id() != b->id())
        && (a->id() != c->id())
        && (b->id() != c->id())
    );
}

TEST_F(GraphTest, AddNode_Templated_ReturnsCorrectType) {
    auto* node = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kLiteral);
    
    // Verify it's actually a LiteralNode by calling a method specific to it
    EXPECT_EQ(node->GetDisplayName(), "Literal");
}

TEST_F(GraphTest, GetNode_ValidId_ReturnsCorrectNode) {
    auto* node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    uint32_t id = node->id();
    
    auto* retrieved = graph_.GetNode(id);
    EXPECT_EQ(retrieved, node);
    EXPECT_EQ(retrieved->id(), id);
}

TEST_F(GraphTest, GetNode_InvalidId_ReturnsNull) {
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    
    // Try to get a node with an ID that doesn't exist
    auto* node = graph_.GetNode(999);
    EXPECT_EQ(node, nullptr);
}

TEST_F(GraphTest, GetNode_Templated_ReturnsCorrectType) {
    auto* added_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    uint32_t id = added_node->id();
    
    auto* retrieved = graph_.GetNode<core::LiteralNode>(id);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->id(), id);
    EXPECT_EQ(retrieved->GetDisplayName(), "Literal");
}

TEST_F(GraphTest, RemoveNode_ValidNode_RemovesFromGraph) {
    auto* node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    uint32_t id = node->id();
    
    graph_.RemoveNode(node);
    
    // Verify node is no longer accessible
    auto* retrieved = graph_.GetNode(id);
    EXPECT_EQ(retrieved, nullptr);
}

TEST_F(GraphTest, RemoveNode_NullPointer_DoesNotCrash) {
    // Should not crash when removing null
    EXPECT_NO_THROW(graph_.RemoveNode(nullptr));
}

TEST_F(GraphTest, RemoveNode_NonExistentNode_DoesNotCrash) {
    // Create a node in a different graph
    core::Graph other_graph;
    auto* other_node = other_graph.AddNode(core::NodeBase::NodeKind::kLiteral);
    
    // Try to remove it from this graph - should not crash
    EXPECT_NO_THROW(graph_.RemoveNode(other_node));
}


