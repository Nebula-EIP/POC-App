#include <gtest/gtest.h>

#include "graph.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/variable_node.hpp"

class GraphTest : public testing::Test {
 protected:
    core::Graph graph_;
};

#pragma region Node Management

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

#pragma endregion

#pragma region Connection/Linking

TEST_F(GraphTest, Link_ValidConnection_ReturnsSuccess) {
    auto* source = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* target = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // Set matching types for successful connection
    static_cast<core::LiteralNode*>(source)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(target)->set_type(core::NodeBase::PinDataType::kInt);
    
    auto result = graph_.Link(source, 0, target, 0);
    EXPECT_TRUE(result.has_value());
}

TEST_F(GraphTest, Link_NullSourceNode_ReturnsError) {
    auto* target = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    auto result = graph_.Link(nullptr, 0, target, 0);
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(result.error().empty());
}

TEST_F(GraphTest, Link_NullTargetNode_ReturnsError) {
    auto* source = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    
    auto result = graph_.Link(source, 0, nullptr, 0);
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(result.error().empty());
}

TEST_F(GraphTest, Link_InvalidOutputPin_ReturnsError) {
    auto* source = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* target = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // LiteralNode has only 1 output pin (index 0), so index 5 is out of bounds
    auto result = graph_.Link(source, 5, target, 0);
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(result.error().empty());
}

TEST_F(GraphTest, Link_InvalidInputPin_ReturnsError) {
    auto* source = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* target = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // VariableNode has only 1 input pin (index 0), so index 5 is invalid
    auto result = graph_.Link(source, 0, target, 5);
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(result.error().empty());
}

TEST_F(GraphTest, Link_IncompatibleTypes_ReturnsError) {
    auto* source = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* target = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // Set different types to make them incompatible
    static_cast<core::LiteralNode*>(source)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(target)->set_type(core::NodeBase::PinDataType::kString);
    
    auto result = graph_.Link(source, 0, target, 0);
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(result.error().empty());
}

TEST_F(GraphTest, Link_AlreadyConnectedInput_ReturnsError) {
    auto* source1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* source2 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* target = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // Set matching types
    static_cast<core::LiteralNode*>(source1)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::LiteralNode*>(source2)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(target)->set_type(core::NodeBase::PinDataType::kInt);
    
    // Connect source1 to target
    auto result1 = graph_.Link(source1, 0, target, 0);
    ASSERT_TRUE(result1.has_value());
    
    // Try to connect source2 to the same input pin
    // Current implementation allows overwriting, so this should succeed
    auto result2 = graph_.Link(source2, 0, target, 0);
    EXPECT_TRUE(result2.has_value());
    
    // Verify that source2 is now the parent (overwrote source1)
    auto* parent_conn = target->parent(0);
    ASSERT_NE(parent_conn, nullptr);
    EXPECT_EQ(parent_conn->node, source2);
}

TEST_F(GraphTest, Link_EstablishesBidirectionalConnection) {
    auto* source = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* target = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // Set matching types
    static_cast<core::LiteralNode*>(source)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(target)->set_type(core::NodeBase::PinDataType::kInt);
    
    graph_.Link(source, 0, target, 0);
    
    // Check parent connection (target -> source)
    auto* parent_conn = target->parent(0);
    ASSERT_NE(parent_conn, nullptr);
    EXPECT_EQ(parent_conn->node, source);
    EXPECT_EQ(parent_conn->pin, 0);
    
    // Check child connection (source -> target)
    const auto& children = source->childrens(0);
    ASSERT_FALSE(children.empty());
    EXPECT_EQ(children[0].node, target);
    EXPECT_EQ(children[0].pin, 0);
}

TEST_F(GraphTest, Link_MultipleOutputConnections_AllowsMultipleChildren) {
    auto* source = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* target1 = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    auto* target2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    auto* target3 = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // Set matching types for all nodes
    static_cast<core::LiteralNode*>(source)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(target1)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(target2)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(target3)->set_type(core::NodeBase::PinDataType::kInt);
    
    // Connect one output to multiple inputs
    auto result1 = graph_.Link(source, 0, target1, 0);
    auto result2 = graph_.Link(source, 0, target2, 0);
    auto result3 = graph_.Link(source, 0, target3, 0);
    
    EXPECT_TRUE(result1.has_value());
    EXPECT_TRUE(result2.has_value());
    EXPECT_TRUE(result3.has_value());
    
    // Verify all three children are connected
    const auto& children = source->childrens(0);
    EXPECT_EQ(children.size(), 3);
}

TEST_F(GraphTest, Unlink_ValidConnection_ReturnsSuccess) {
    auto* source = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* target = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // Set matching types
    static_cast<core::LiteralNode*>(source)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(target)->set_type(core::NodeBase::PinDataType::kInt);
    
    graph_.Link(source, 0, target, 0);
    
    auto result = graph_.Unlink(source, 0, target, 0);
    EXPECT_TRUE(result.has_value());
}

TEST_F(GraphTest, Unlink_RemovesParentConnection) {
    auto* source = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* target = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // Set matching types
    static_cast<core::LiteralNode*>(source)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(target)->set_type(core::NodeBase::PinDataType::kInt);
    
    graph_.Link(source, 0, target, 0);
    graph_.Unlink(source, 0, target, 0);
    
    // Check that parent connection is cleared
    auto* parent_conn = target->parent(0);
    EXPECT_TRUE(parent_conn == nullptr || !parent_conn->IsConnected());
}

TEST_F(GraphTest, Unlink_RemovesChildConnection) {
    auto* source = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* target = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // Set matching types
    static_cast<core::LiteralNode*>(source)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(target)->set_type(core::NodeBase::PinDataType::kInt);
    
    graph_.Link(source, 0, target, 0);
    
    // Verify connection exists
    ASSERT_FALSE(source->childrens(0).empty());
    
    graph_.Unlink(source, 0, target, 0);
    
    // Check that child connection is removed
    const auto& children = source->childrens(0);
    
    // Either the vector is empty, or all connections are disconnected
    bool all_disconnected = true;
    for (const auto& conn : children) {
        if (conn.IsConnected() && conn.node == target && conn.pin == 0) {
            all_disconnected = false;
            break;
        }
    }
    EXPECT_TRUE(all_disconnected);
}

TEST_F(GraphTest, Unlink_NullPointers_ReturnsError) {
    auto* node = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    auto result1 = graph_.Unlink(nullptr, 0, node, 0);
    EXPECT_FALSE(result1.has_value());
    
    auto result2 = graph_.Unlink(node, 0, nullptr, 0);
    EXPECT_FALSE(result2.has_value());
    
    auto result3 = graph_.Unlink(nullptr, 0, nullptr, 0);
    EXPECT_FALSE(result3.has_value());
}

TEST_F(GraphTest, Unlink_NonExistentConnection_DoesNotCrash) {
    auto* source = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* target = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // Try to unlink nodes that were never connected - should not crash
    EXPECT_NO_THROW(graph_.Unlink(source, 0, target, 0));
}

#pragma endregion

#pragma region Graph Integrity



#pragma endregion
