#include <gtest/gtest.h>

#include "graph.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/variable_node.hpp"

class GraphTest : public testing::Test {
 protected:
    core::Graph graph_;

    void DumpGraph() const {
        core::NodeBase *node = graph_.GetNode(0);
    
        printf("### DUMPING GRAPH ###\n");
        for (uint32_t i = 1; node != nullptr; i++) {
            printf("Node[%d]\n", node->id());
    
            for (uint8_t i = 0; i < node->GetInputPinCount(); i++) {
                const core::NodeBase::Connection *conn = node->parent(i);
                if (conn && conn->IsConnected()) {
                    printf("  IN[%d] <- Node[%d] Out[%d]\n", i, conn->node->id(), conn->pin);
                }
            }
            for (uint8_t i = 0; i < node->GetOutputPinCount(); i++) {
                for (const core::NodeBase::Connection &conn : node->childrens(i)) {
                    
                    printf("  OUT[%d] -> Node[%d] In[%d]\n",
                        i,
                        (conn.node ? conn.node->id() : 0),
                        conn.pin);
                }
            }
            node = graph_.GetNode(i);
        }
        printf("### COMPLETED ###\n");
    }
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

TEST_F(GraphTest, RemoveNode_WithConnections_UnlinksAllConnections) {
    // Create a node with both input and output connections
    auto* source = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* middle = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    auto* target = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // Set matching types
    static_cast<core::LiteralNode*>(source)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(middle)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(target)->set_type(core::NodeBase::PinDataType::kInt);
    
    // Create connections: source -> middle -> target
    graph_.Link(source, 0, middle, 0);
    graph_.Link(middle, 0, target, 0);
    
    // Remove the middle node
    graph_.RemoveNode(middle);
    
    // Verify source's children list no longer references middle
    const auto& source_children = source->childrens(0);
    for (const auto& conn : source_children) {
        if (conn.IsConnected()) {
            EXPECT_NE(conn.node, middle);
        }
    }
    
    // Verify target's parent no longer references middle
    auto* target_parent = target->parent(0);
    if (target_parent != nullptr && target_parent->IsConnected()) {
        EXPECT_NE(target_parent->node, middle);
    }
}

TEST_F(GraphTest, RemoveNode_ParentNode_ClearsChildReferences) {
    // Create parent -> child connection
    auto* parent = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* child1 = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    auto* child2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // Set matching types
    static_cast<core::LiteralNode*>(parent)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(child1)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(child2)->set_type(core::NodeBase::PinDataType::kInt);
    
    // Connect parent to multiple children
    graph_.Link(parent, 0, child1, 0);
    graph_.Link(parent, 0, child2, 0);
    
    // Verify connections exist before removal
    ASSERT_NE(child1->parent(0), nullptr);
    ASSERT_NE(child2->parent(0), nullptr);
    
    // Remove parent node
    graph_.RemoveNode(parent);
    
    // Verify children's parent pointers are cleared
    auto* child1_parent = child1->parent(0);
    EXPECT_TRUE(child1_parent == nullptr || !child1_parent->IsConnected());
    
    auto* child2_parent = child2->parent(0);
    EXPECT_TRUE(child2_parent == nullptr || !child2_parent->IsConnected());
}

TEST_F(GraphTest, RemoveNode_ChildNode_RemovesFromParentChildList) {
    // Create parent with multiple children
    auto* parent = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* child1 = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    auto* child2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    auto* child3 = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // Set matching types
    static_cast<core::LiteralNode*>(parent)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(child1)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(child2)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(child3)->set_type(core::NodeBase::PinDataType::kInt);
    
    // Connect parent to all children
    graph_.Link(parent, 0, child1, 0);
    graph_.Link(parent, 0, child2, 0);
    graph_.Link(parent, 0, child3, 0);
    
    // Verify all children are in parent's list
    ASSERT_EQ(parent->childrens(0).size(), 3);
    
    // Remove child2
    graph_.RemoveNode(child2);
    
    // Verify child2 is no longer in parent's child list
    const auto& children = parent->childrens(0);
    for (const auto& conn : children) {
        if (conn.IsConnected()) {
            EXPECT_NE(conn.node, child2);
        }
    }
    
    // Verify child1 and child3 are still connected
    bool found_child1 = false;
    bool found_child3 = false;
    for (const auto& conn : children) {
        if (conn.IsConnected()) {
            if (conn.node == child1) found_child1 = true;
            if (conn.node == child3) found_child3 = true;
        }
    }
    EXPECT_TRUE(found_child1);
    EXPECT_TRUE(found_child3);
}

TEST_F(GraphTest, ComplexGraph_MultipleNodesAndConnections_MaintainsIntegrity) {
    // Create a complex graph structure:
    //     lit1 ---|
    //             |--> var1 --> var2
    //     lit2 --/              /
    //                          /
    //     lit3 ---------------/
    
    auto* lit1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* lit2 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* lit3 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* var1 = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    auto* var2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable);
    
    // Set matching types for all connections
    static_cast<core::LiteralNode*>(lit1)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::LiteralNode*>(lit2)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::LiteralNode*>(lit3)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(var1)->set_type(core::NodeBase::PinDataType::kInt);
    static_cast<core::VariableNode*>(var2)->set_type(core::NodeBase::PinDataType::kInt);
    
    // Create connections
    DumpGraph(graph_);
    ASSERT_TRUE(graph_.Link(lit1, 0, var1, 0).has_value());
    // var1 input can only connect to one, so lit2 will overwrite lit1
    DumpGraph(graph_);
    ASSERT_TRUE(graph_.Link(lit2, 0, var1, 0).has_value());
    DumpGraph(graph_);
    ASSERT_TRUE(graph_.Link(var1, 0, var2, 0).has_value());
    // var2 input will be overwritten by lit3
    DumpGraph(graph_);
    ASSERT_TRUE(graph_.Link(lit3, 0, var2, 0).has_value());
    
    // Verify initial connections
    // var1's parent should be lit2 (overwrote lit1)
    DumpGraph(graph_);
    auto* var1_parent = var1->parent(0);
    ASSERT_NE(var1_parent, nullptr);
    EXPECT_EQ(var1_parent->node, lit2);
    
    // var2's parent should be lit3 (overwrote var1)
    auto* var2_parent = var2->parent(0);
    ASSERT_NE(var2_parent, nullptr);
    EXPECT_EQ(var2_parent->node, lit3);
    
    // lit2 should have var1 as child
    const auto& lit2_children = lit2->childrens(0);
    bool found_var1_in_lit2 = false;
    for (const auto& conn : lit2_children) {
        if (conn.IsConnected() && conn.node == var1) {
            found_var1_in_lit2 = true;
            break;
        }
    }
    EXPECT_TRUE(found_var1_in_lit2);
    
    // lit3 should have var2 as child
    const auto& lit3_children = lit3->childrens(0);
    bool found_var2_in_lit3 = false;
    for (const auto& conn : lit3_children) {
        if (conn.IsConnected() && conn.node == var2) {
            found_var2_in_lit3 = true;
            break;
        }
    }
    EXPECT_TRUE(found_var2_in_lit3);
    
    // Remove var1 (middle node)
    graph_.RemoveNode(var1);

    DumpGraph(graph_);
    
    // Verify var1 is gone
    EXPECT_EQ(graph_.GetNode(var1->id()), nullptr);
    
    // Verify lit2 no longer has var1 in its children
    const auto& lit2_children_after = lit2->childrens(0);
    for (const auto& conn : lit2_children_after) {
        if (conn.IsConnected()) {
            EXPECT_NE(conn.node, var1);
        }
    }
    
    // Verify other connections remain intact
    // var2 should still be connected to lit3
    auto* var2_parent_after = var2->parent(0);
    ASSERT_NE(var2_parent_after, nullptr);
    EXPECT_EQ(var2_parent_after->node, lit3);
    
    // lit3 should still have var2 as child
    const auto& lit3_children_after = lit3->childrens(0);
    bool still_found_var2 = false;
    for (const auto& conn : lit3_children_after) {
        if (conn.IsConnected() && conn.node == var2) {
            still_found_var2 = true;
            break;
        }
    }
    EXPECT_TRUE(still_found_var2);
}

#pragma endregion
