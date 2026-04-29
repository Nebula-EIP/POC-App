#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

#include "graph.hpp"
#include "graph_exceptions.hpp"
#include "connection_exceptions.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/variable_node.hpp"
#include "nodes/function_node.hpp"
#include "nodes/function_input_node.hpp"
#include "nodes/function_output_node.hpp"
#include "utils/logger.hpp"

class GraphTest : public testing::Test {
 protected:
    core::Graph graph_;
};

static void DumpGraph(const core::Graph &graph) {
    printf("### DUMPING GRAPH ###\n");
    for (const auto &node : graph.GetAllNodes()) {
        printf("Node[%d:%s]\n", node->id(), core::NodeKindToString(node->kind()).c_str());

        printf("  Inputs: %ld/%d\n", node->GetAllParents().size(), node->GetInputPinCount());
        for (auto conn : node->GetAllParents()) {
            if (conn.IsConnected()) {
                printf("    IN[%d:%s] <- Node[%d:%s] Out[%d:%s]\n",
                    conn.in_pin, core::PinDataTypeToString(conn.type).c_str(),
                    conn.node->id(), node->GetDisplayName().c_str(),
                    conn.out_pin, core::PinDataTypeToString(conn.type).c_str());
            } else {
                printf("    IN[%d:%s] <- Disconnected\n",
                    conn.in_pin, core::PinDataTypeToString(conn.type).c_str());
            }
        }

        printf("  Outputs: %ld/%d\n", node->GetAllChildrens().size(), node->GetOutputPinCount());
        for (auto conn : node->GetAllChildrens()) {
            if (conn.IsConnected())  {
                printf("    OUT[%d:%s] -> Node[%d:%s] In[%d:%s]\n",
                    conn.out_pin, core::PinDataTypeToString(conn.type).c_str(),
                    conn.node->id(), core::NodeKindToString(conn.node->kind()).c_str(),
                    conn.in_pin, core::PinDataTypeToString(conn.type).c_str());
            } else {
                printf("    OUT[%d:%s] -> Disconnected\n",
                    conn.out_pin, core::PinDataTypeToString(conn.type).c_str());
            }
        }
    }
    printf("### COMPLETED ###\n");
}


/* ################################################################ */
/* #################### Constructor & Metadata #################### */
/* ################################################################ */

#pragma region Constructor & Metadata

TEST_F(GraphTest, ConstructorInitializesDefaultValues) {
    // Verify default project name
    EXPECT_EQ(graph_.GetProjectName(), "Untitled Project");
    
    // Verify default version
    EXPECT_EQ(graph_.GetVersion(), "1.0");
    
    // Verify empty author
    EXPECT_EQ(graph_.GetAuthor(), "");
    
    // Verify timestamps are set to current time (within a reasonable margin)
    auto now = std::chrono::system_clock::now();
    auto created = graph_.GetCreatedAt();
    auto modified = graph_.GetModifiedAt();
    
    // Check that timestamps are within 1 second of now
    auto created_diff = std::chrono::duration_cast<std::chrono::seconds>(now - created).count();
    auto modified_diff = std::chrono::duration_cast<std::chrono::seconds>(now - modified).count();
    
    EXPECT_LE(std::abs(created_diff), 1);
    EXPECT_LE(std::abs(modified_diff), 1);
    
    // Created and modified should be equal (or very close) at construction
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(modified - created).count();
    EXPECT_LE(std::abs(diff), 100);
}

TEST_F(GraphTest, SetProjectNameUpdatesName) {
    const std::string new_name = "My Test Project";
    
    graph_.SetProjectName(new_name);
    
    EXPECT_EQ(graph_.GetProjectName(), new_name);
}

TEST_F(GraphTest, SetProjectNameUpdatesModifiedTime) {
    auto initial_modified = graph_.GetModifiedAt();
    
    // Sleep briefly to ensure time difference
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    graph_.SetProjectName("Updated Project");
    
    auto new_modified = graph_.GetModifiedAt();
    
    // Modified time should be greater than initial
    EXPECT_GT(new_modified, initial_modified);
}

TEST_F(GraphTest, SetAuthorUpdatesAuthor) {
    const std::string author_name = "John Doe";
    
    graph_.SetAuthor(author_name);
    
    EXPECT_EQ(graph_.GetAuthor(), author_name);
}

TEST_F(GraphTest, SetAuthorUpdatesModifiedTime) {
    auto initial_modified = graph_.GetModifiedAt();
    
    // Sleep briefly to ensure time difference
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    graph_.SetAuthor("Jane Smith");
    
    auto new_modified = graph_.GetModifiedAt();
    
    // Modified time should be greater than initial
    EXPECT_GT(new_modified, initial_modified);
}

TEST_F(GraphTest, GettersReturnCorrectMetadata) {
    // Set all metadata fields
    const std::string project_name = "Test Project";
    const std::string author = "Test Author";
    
    graph_.SetProjectName(project_name);
    graph_.SetAuthor(author);
    
    // Verify all getters return correct values
    EXPECT_EQ(graph_.GetProjectName(), project_name);
    EXPECT_EQ(graph_.GetVersion(), "1.0");
    EXPECT_EQ(graph_.GetAuthor(), author);
    
    // Verify timestamps are valid
    auto created = graph_.GetCreatedAt();
    auto modified = graph_.GetModifiedAt();
    
    // Modified should be after or equal to created
    EXPECT_GE(modified, created);
    
    // Verify timestamps are reasonable (not in the future)
    auto now = std::chrono::system_clock::now();
    EXPECT_LE(created, now);
    EXPECT_LE(modified, now);
}

#pragma endregion Constructor & Metadata
/* ################################################# */
/* #################### AddNode #################### */
/* ################################################# */

#pragma region AddNode

TEST_F(GraphTest, AddNodeCreatesLiteralNode) {
    auto *node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kLiteral);
    EXPECT_EQ(node->id(), 0);
}

TEST_F(GraphTest, AddNodeCreatesVariableNode) {
    auto *node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kVariable);
    EXPECT_EQ(node->id(), 0);
}

TEST_F(GraphTest, AddNodeCreatesFunctionNode) {
    auto *node = graph_.AddNode(core::NodeBase::NodeKind::kFunction, {0, 0});
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kFunction);
    EXPECT_EQ(node->id(), 0);
}

TEST_F(GraphTest, AddNodeCreatesFunctionInputNode) {
    auto *node = graph_.AddNode(core::NodeBase::NodeKind::kFunctionInput, {0, 0});
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kFunctionInput);
    EXPECT_EQ(node->id(), 0);
}

TEST_F(GraphTest, AddNodeCreatesFunctionOutputNode) {
    auto *node = graph_.AddNode(core::NodeBase::NodeKind::kFunctionOutput, {0, 0});
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kFunctionOutput);
    EXPECT_EQ(node->id(), 0);
}

TEST_F(GraphTest, AddNodeThrowsOnUndefinedKind) {
    EXPECT_THROW({
        graph_.AddNode(core::NodeBase::NodeKind::kUndefined, {0, 0});
    }, core::InvalidNodeKindException);
}

TEST_F(GraphTest, AddNodeAssignsUniqueIds) {
    auto *node1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *node2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *node3 = graph_.AddNode(core::NodeBase::NodeKind::kFunction, {0, 0});
    
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    ASSERT_NE(node3, nullptr);
    
    // Verify IDs are unique
    EXPECT_EQ(node1->id(), 0);
    EXPECT_EQ(node2->id(), 1);
    EXPECT_EQ(node3->id(), 2);
    
    // Verify all IDs are different
    EXPECT_NE(node1->id(), node2->id());
    EXPECT_NE(node2->id(), node3->id());
    EXPECT_NE(node1->id(), node3->id());
}

TEST_F(GraphTest, AddNodeTemplatedVersionReturnsCorrectType) {
    // Test with LiteralNode
    auto *literal_node = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    ASSERT_NE(literal_node, nullptr);
    EXPECT_EQ(literal_node->kind(), core::NodeBase::NodeKind::kLiteral);
    
    // Test with VariableNode
    auto *variable_node = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable, {0, 0});
    ASSERT_NE(variable_node, nullptr);
    EXPECT_EQ(variable_node->kind(), core::NodeBase::NodeKind::kVariable);
    
    // Test with FunctionNode
    auto *function_node = graph_.AddNode<core::FunctionNode>(core::NodeBase::NodeKind::kFunction, {0, 0});
    ASSERT_NE(function_node, nullptr);
    EXPECT_EQ(function_node->kind(), core::NodeBase::NodeKind::kFunction);
    
    // Test with FunctionInputNode
    auto *input_node = graph_.AddNode<core::FunctionInputNode>(core::NodeBase::NodeKind::kFunctionInput, {0, 0});
    ASSERT_NE(input_node, nullptr);
    EXPECT_EQ(input_node->kind(), core::NodeBase::NodeKind::kFunctionInput);
    
    // Test with FunctionOutputNode
    auto *output_node = graph_.AddNode<core::FunctionOutputNode>(core::NodeBase::NodeKind::kFunctionOutput, {0, 0});
    ASSERT_NE(output_node, nullptr);
    EXPECT_EQ(output_node->kind(), core::NodeBase::NodeKind::kFunctionOutput);
}

#pragma endregion AddNode

/* #################################################### */
/* #################### RemoveNode #################### */
/* #################################################### */

#pragma region RemoveNode

TEST_F(GraphTest, RemoveNodeDeletesExistingNode) {
    auto *node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    ASSERT_NE(node, nullptr);
    
    uint32_t node_id = node->id();
    
    // Remove the node
    graph_.RemoveNode(node);
    
    // Verify node no longer exists in the graph
    EXPECT_EQ(graph_.GetNode(node_id), nullptr);
}

TEST_F(GraphTest, RemoveNodeThrowsOnNullptr) {
    EXPECT_THROW({
        graph_.RemoveNode(nullptr);
    }, core::NodeNotFoundException);
}

TEST_F(GraphTest, RemoveNodeThrowsOnUnownedNode) {
    // Create a separate graph with its own node
    core::Graph other_graph;
    auto *other_node = other_graph.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    ASSERT_NE(other_node, nullptr);
    
    // Try to remove the other graph's node from this graph
    EXPECT_THROW({
        graph_.RemoveNode(other_node);
    }, core::NodeNotFoundException);
}

TEST_F(GraphTest, RemoveNodeDisconnectsParentConnections) {
    // Create three nodes: parent -> middle -> child
    auto *parent = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *middle = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *child = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(middle, nullptr);
    ASSERT_NE(child, nullptr);
    
    // Link parent -> middle
    graph_.Link(parent, 0, middle, 0);
    // Verify middle is connected to parent
    EXPECT_TRUE(middle->IsInputPinConnected(0));
    
    // Remove middle node
    graph_.RemoveNode(middle);
    // Verify parent no longer has middle as a child
    EXPECT_FALSE(parent->IsOutputPinConnected(0));

}

TEST_F(GraphTest, RemoveNodeDisconnectsChildConnections) {
    // Create three nodes: parent -> middle -> child
    auto *parent = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *middle = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *child = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(middle, nullptr);
    ASSERT_NE(child, nullptr);
    
    // Link middle -> child
    graph_.Link(middle, 0, child, 0);
    
    // Verify child is connected to middle
    EXPECT_TRUE(child->IsInputPinConnected(0));
    
    // Remove middle node
    graph_.RemoveNode(middle);
    
    // Verify child no longer has middle as a parent
    EXPECT_FALSE(child->IsInputPinConnected(0));
}

TEST_F(GraphTest, RemoveNodeFreesNodeId) {
    // Add a node
    auto *node1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    ASSERT_NE(node1, nullptr);
    uint32_t first_id = node1->id();
    EXPECT_EQ(first_id, 0);
    
    // Add another node
    auto *node2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    ASSERT_NE(node2, nullptr);
    EXPECT_EQ(node2->id(), 1);
    
    // Remove the first node
    graph_.RemoveNode(node1);
    
    // Add a new node - it should reuse the freed ID
    auto *node3 = graph_.AddNode(core::NodeBase::NodeKind::kFunction, {0, 0});
    ASSERT_NE(node3, nullptr);
    
    // The new node should have reused the freed ID (0)
    EXPECT_EQ(node3->id(), first_id);
}

#pragma endregion RemoveNode

#pragma region GetNode

TEST_F(GraphTest, GetNodeReturnsExistingNode) {
    // Add nodes with different kinds
    auto *literal_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *variable_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *function_node = graph_.AddNode(core::NodeBase::NodeKind::kFunction, {0, 0});
    
    ASSERT_NE(literal_node, nullptr);
    ASSERT_NE(variable_node, nullptr);
    ASSERT_NE(function_node, nullptr);
    
    uint32_t literal_id = literal_node->id();
    uint32_t variable_id = variable_node->id();
    uint32_t function_id = function_node->id();
    
    // Retrieve nodes by their IDs
    auto *retrieved_literal = graph_.GetNode(literal_id);
    auto *retrieved_variable = graph_.GetNode(variable_id);
    auto *retrieved_function = graph_.GetNode(function_id);
    
    // Verify we get the same nodes back
    EXPECT_EQ(retrieved_literal, literal_node);
    EXPECT_EQ(retrieved_variable, variable_node);
    EXPECT_EQ(retrieved_function, function_node);
    
    // Verify node properties are correct
    EXPECT_EQ(retrieved_literal->kind(), core::NodeBase::NodeKind::kLiteral);
    EXPECT_EQ(retrieved_variable->kind(), core::NodeBase::NodeKind::kVariable);
    EXPECT_EQ(retrieved_function->kind(), core::NodeBase::NodeKind::kFunction);
}

TEST_F(GraphTest, GetNodeReturnsNullptrForNonexistentId) {
    // Try to get a node with an ID that doesn't exist
    auto *node = graph_.GetNode(999);
    
    EXPECT_EQ(node, nullptr);
    
    // Add some nodes
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    // Try to get a node with an ID that still doesn't exist
    node = graph_.GetNode(100);
    
    EXPECT_EQ(node, nullptr);
}

TEST_F(GraphTest, GetNodeTemplatedVersionReturnsCorrectType) {
    // Add nodes of specific types
    auto *literal_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *variable_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *function_node = graph_.AddNode(core::NodeBase::NodeKind::kFunction, {0, 0});
    auto *input_node = graph_.AddNode(core::NodeBase::NodeKind::kFunctionInput, {0, 0});
    auto *output_node = graph_.AddNode(core::NodeBase::NodeKind::kFunctionOutput, {0, 0});
    
    ASSERT_NE(literal_node, nullptr);
    ASSERT_NE(variable_node, nullptr);
    ASSERT_NE(function_node, nullptr);
    ASSERT_NE(input_node, nullptr);
    ASSERT_NE(output_node, nullptr);
    
    // Retrieve nodes using templated version
    auto *retrieved_literal = graph_.GetNode<core::LiteralNode>(literal_node->id());
    auto *retrieved_variable = graph_.GetNode<core::VariableNode>(variable_node->id());
    auto *retrieved_function = graph_.GetNode<core::FunctionNode>(function_node->id());
    auto *retrieved_input = graph_.GetNode<core::FunctionInputNode>(input_node->id());
    auto *retrieved_output = graph_.GetNode<core::FunctionOutputNode>(output_node->id());
    
    // Check for nulls
    ASSERT_NE(retrieved_literal, nullptr);
    ASSERT_NE(retrieved_variable, nullptr);
    ASSERT_NE(retrieved_function, nullptr);
    ASSERT_NE(retrieved_input, nullptr);
    ASSERT_NE(retrieved_output, nullptr);
    
    // Verify the returned pointers match and have correct kinds
    EXPECT_EQ(retrieved_literal->kind(), core::NodeBase::NodeKind::kLiteral);
    EXPECT_EQ(retrieved_variable->kind(), core::NodeBase::NodeKind::kVariable);
    EXPECT_EQ(retrieved_function->kind(), core::NodeBase::NodeKind::kFunction);
    EXPECT_EQ(retrieved_input->kind(), core::NodeBase::NodeKind::kFunctionInput);
    EXPECT_EQ(retrieved_output->kind(), core::NodeBase::NodeKind::kFunctionOutput);
    
    // Test that nullptr is returned for non-existent ID
    auto *null_node = graph_.GetNode<core::LiteralNode>(999);
    EXPECT_EQ(null_node, nullptr);
}

#pragma endregion GetNode

#pragma region Link

TEST_F(GraphTest, LinkConnectsTwoNodes) {
    // Create two nodes with compatible pin types
    auto *from_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *to_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(from_node, nullptr);
    ASSERT_NE(to_node, nullptr);
    
    // Verify nodes are not connected initially
    EXPECT_FALSE(to_node->IsInputPinConnected(0));
    EXPECT_FALSE(from_node->IsOutputPinConnected(0));
    
    // Link the nodes
    graph_.Link(from_node, 0, to_node, 0);
    
    // Verify connection was established
    EXPECT_TRUE(to_node->IsInputPinConnected(0));
    EXPECT_TRUE(from_node->IsOutputPinConnected(0));
}

TEST_F(GraphTest, LinkThrowsOnNullFromNode) {
    auto *to_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    ASSERT_NE(to_node, nullptr);
    
    EXPECT_THROW({
        graph_.Link(nullptr, 0, to_node, 0);
    }, core::NodeNotFoundException);
}

TEST_F(GraphTest, LinkThrowsOnNullToNode) {
    auto *from_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    ASSERT_NE(from_node, nullptr);
    
    EXPECT_THROW({
        graph_.Link(from_node, 0, nullptr, 0);
    }, core::NodeNotFoundException);
}

TEST_F(GraphTest, LinkThrowsOnUnownedFromNode) {
    // Create a node in another graph
    core::Graph other_graph;
    auto *from_node = other_graph.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    ASSERT_NE(from_node, nullptr);
    
    // Create a node in this graph
    auto *to_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    ASSERT_NE(to_node, nullptr);
    
    // Try to link from unowned node
    EXPECT_THROW({
        graph_.Link(from_node, 0, to_node, 0);
    }, core::NodeNotFoundException);
}

TEST_F(GraphTest, LinkThrowsOnUnownedToNode) {
    // Create a node in this graph
    auto *from_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    ASSERT_NE(from_node, nullptr);
    
    // Create a node in another graph
    core::Graph other_graph;
    auto *to_node = other_graph.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    ASSERT_NE(to_node, nullptr);
    
    // Try to link to unowned node
    EXPECT_THROW({
        graph_.Link(from_node, 0, to_node, 0);
    }, core::NodeNotFoundException);
}

TEST_F(GraphTest, LinkThrowsOnInvalidOutputPin) {
    auto *from_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *to_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(from_node, nullptr);
    ASSERT_NE(to_node, nullptr);
    
    // Try to link with invalid output pin (out of bounds)
    uint8_t invalid_pin = from_node->GetOutputPinCount() + 10;
    
    EXPECT_THROW({
        graph_.Link(from_node, invalid_pin, to_node, 0);
    }, core::InvalidPinIndexException);
}

TEST_F(GraphTest, LinkThrowsOnInvalidInputPin) {
    auto *from_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *to_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(from_node, nullptr);
    ASSERT_NE(to_node, nullptr);
    
    // Try to link with invalid input pin (out of bounds)
    uint8_t invalid_pin = to_node->GetInputPinCount() + 10;
    
    EXPECT_THROW({
        graph_.Link(from_node, 0, to_node, invalid_pin);
    }, core::InvalidPinIndexException);
}

TEST_F(GraphTest, LinkThrowsOnIncompatiblePinTypes) {
    // Create two nodes with incompatible types
    // LiteralNode typically has one type, we need nodes with different pin types
    auto *literal_node = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *variable_node = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(literal_node, nullptr);
    ASSERT_NE(variable_node, nullptr);
    
    // Set literal to Int type
    literal_node->set_data(42.0f);
    
    // Set variable to Float type (incompatible with Int)
    variable_node->SetType(core::NodeBase::PinDataType::kFloat);
    
    // Try to link incompatible types
    EXPECT_THROW({
        graph_.Link(literal_node, 0, variable_node, 0);
    }, core::IncompatiblePinTypesException);
}

TEST_F(GraphTest, LinkThrowsOnCircularDependency) {
    auto *node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    ASSERT_NE(node, nullptr);
    
    // Try to link a node to itself
    EXPECT_THROW({
        graph_.Link(node, 0, node, 0);
    }, core::CircularDependencyException);
}

TEST_F(GraphTest, LinkSeversPreviousConnection) {
    // Create three nodes: A -> B, then A' -> B (severing A -> B)
    auto *node_a = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *node_a_prime = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *node_b = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(node_a, nullptr);
    ASSERT_NE(node_a_prime, nullptr);
    ASSERT_NE(node_b, nullptr);
    
    // Link A -> B
    graph_.Link(node_a, 0, node_b, 0);
    
    // Verify connection exists
    EXPECT_TRUE(node_b->IsInputPinConnected(0));
    auto conn = node_b->parent(0);
    EXPECT_EQ(conn.node, node_a);
    
    // Link A' -> B (should sever A -> B)
    graph_.Link(node_a_prime, 0, node_b, 0);
    
    // Verify new connection
    EXPECT_TRUE(node_b->IsInputPinConnected(0));
    conn = node_b->parent(0);
    EXPECT_EQ(conn.node, node_a_prime);
    EXPECT_NE(conn.node, node_a);
    
    // Verify old connection was severed
    EXPECT_FALSE(node_a->IsOutputPinConnected(0));
}

TEST_F(GraphTest, LinkUpdatesParentConnection) {
    auto *from_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *to_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(from_node, nullptr);
    ASSERT_NE(to_node, nullptr);
    
    // Link the nodes
    graph_.Link(from_node, 0, to_node, 0);
    
    // Verify parent connection is set correctly
    EXPECT_TRUE(to_node->IsInputPinConnected(0));
    
    auto parent_conn = to_node->parent(0);
    EXPECT_EQ(parent_conn.node, from_node);
    EXPECT_EQ(parent_conn.out_pin, 0);
    EXPECT_EQ(parent_conn.in_pin, 0);
    EXPECT_TRUE(parent_conn.IsConnected());
}

TEST_F(GraphTest, LinkUpdatesChildConnection) {
    auto *from_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *to_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(from_node, nullptr);
    ASSERT_NE(to_node, nullptr);
    
    // Link the nodes
    graph_.Link(from_node, 0, to_node, 0);
    
    // Verify child connection is set correctly
    EXPECT_TRUE(from_node->IsOutputPinConnected(0));
    
    auto children = from_node->Childrens(0);
    ASSERT_NE(children, nullptr);
    ASSERT_FALSE(children->empty());
    
    const auto &child_conn = children->at(0);
    EXPECT_EQ(child_conn.node, to_node);
    EXPECT_EQ(child_conn.out_pin, 0);
    EXPECT_EQ(child_conn.in_pin, 0);
    EXPECT_TRUE(child_conn.IsConnected());
}

#pragma endregion Link

#pragma region Unlink

TEST_F(GraphTest, UnlinkDisconnectsTwoNodes) {
    // Create and link two nodes
    auto *from_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *to_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(from_node, nullptr);
    ASSERT_NE(to_node, nullptr);
    
    // Link the nodes
    graph_.Link(from_node, 0, to_node, 0);
    
    // Verify connection exists
    EXPECT_TRUE(to_node->IsInputPinConnected(0));
    EXPECT_TRUE(from_node->IsOutputPinConnected(0));
    
    // Unlink the nodes
    graph_.Unlink(from_node, 0, to_node, 0);
    
    // Verify connection was removed
    EXPECT_FALSE(to_node->IsInputPinConnected(0));
    EXPECT_FALSE(from_node->IsOutputPinConnected(0));
}

TEST_F(GraphTest, UnlinkThrowsOnNullFromNode) {
    auto *to_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    ASSERT_NE(to_node, nullptr);
    
    EXPECT_THROW({
        graph_.Unlink(nullptr, 0, to_node, 0);
    }, core::NodeNotFoundException);
}

TEST_F(GraphTest, UnlinkThrowsOnNullToNode) {
    auto *from_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    ASSERT_NE(from_node, nullptr);
    
    EXPECT_THROW({
        graph_.Unlink(from_node, 0, nullptr, 0);
    }, core::NodeNotFoundException);
}



TEST_F(GraphTest, UnlinkThrowsOnInvalidOutputPin) {
    auto *from_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *to_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(from_node, nullptr);
    ASSERT_NE(to_node, nullptr);
    
    // Link the nodes first
    graph_.Link(from_node, 0, to_node, 0);
    
    // Try to unlink with invalid output pin (out of bounds)
    uint8_t invalid_pin = from_node->GetOutputPinCount() + 10;
    
    EXPECT_THROW({
        graph_.Unlink(from_node, invalid_pin, to_node, 0);
    }, core::InvalidPinIndexException);
}

TEST_F(GraphTest, UnlinkThrowsOnInvalidInputPin) {
    auto *from_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *to_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(from_node, nullptr);
    ASSERT_NE(to_node, nullptr);
    
    // Link the nodes first
    graph_.Link(from_node, 0, to_node, 0);
    
    // Try to unlink with invalid input pin (out of bounds)
    uint8_t invalid_pin = to_node->GetInputPinCount() + 10;
    
    EXPECT_THROW({
        graph_.Unlink(from_node, 0, to_node, invalid_pin);
    }, core::InvalidPinIndexException);
}

TEST_F(GraphTest, UnlinkClearsParentConnection) {
    // Create and link two nodes
    auto *from_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *to_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(from_node, nullptr);
    ASSERT_NE(to_node, nullptr);
    
    // Link the nodes
    graph_.Link(from_node, 0, to_node, 0);
    
    // Verify parent connection exists
    EXPECT_TRUE(to_node->IsInputPinConnected(0));
    auto parent_conn = to_node->parent(0);
    EXPECT_EQ(parent_conn.node, from_node);
    
    // Unlink the nodes
    graph_.Unlink(from_node, 0, to_node, 0);
    
    // Verify parent connection is cleared
    EXPECT_FALSE(to_node->IsInputPinConnected(0));
}

TEST_F(GraphTest, UnlinkRemovesChildConnection) {
    // Create and link two nodes
    auto *from_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *to_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(from_node, nullptr);
    ASSERT_NE(to_node, nullptr);
    
    // Link the nodes
    graph_.Link(from_node, 0, to_node, 0);
    
    // Verify child connection exists
    EXPECT_TRUE(from_node->IsOutputPinConnected(0));
    auto children = from_node->Childrens(0);
    ASSERT_NE(children, nullptr);
    ASSERT_FALSE(children->empty());
    EXPECT_EQ(children->at(0).node, to_node);
    
    // Unlink the nodes
    graph_.Unlink(from_node, 0, to_node, 0);
    
    // Verify child connection is removed
    EXPECT_FALSE(from_node->IsOutputPinConnected(0));
}

#pragma endregion Unlink

#pragma region Serialization

TEST_F(GraphTest, SerializeCreatesValidJson) {
    try {
    // Add some nodes to the graph
    auto *node1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *node2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    
    // Link the nodes
    graph_.Link(node1, 0, node2, 0);
    
    // Serialize the graph
    nlohmann::json json = graph_.Serialize();
    
    // Verify JSON structure contains all required top-level fields
    EXPECT_TRUE(json.contains("metadata"));
    EXPECT_TRUE(json.contains("graph"));
    
    // Verify metadata section
    EXPECT_TRUE(json["metadata"].contains("project_name"));
    EXPECT_TRUE(json["metadata"].contains("version"));
    EXPECT_TRUE(json["metadata"].contains("author"));
    EXPECT_TRUE(json["metadata"].contains("created_at"));
    EXPECT_TRUE(json["metadata"].contains("modified_at"));
    
    // Verify graph section
    EXPECT_TRUE(json["graph"].contains("next_id"));
    EXPECT_TRUE(json["graph"].contains("nodes"));
    EXPECT_TRUE(json["graph"].contains("connections"));
    
    // Verify arrays are actually arrays
    EXPECT_TRUE(json["graph"]["nodes"].is_array());
    EXPECT_TRUE(json["graph"]["connections"].is_array());
    } catch (utils::BaseException &e) {
        LOG_FATAL(e.GetDetailedMessage());
    }
}

TEST_F(GraphTest, SerializeIncludesAllMetadata) {
    // Set custom metadata
    const std::string project_name = "Test Serialization Project";
    const std::string author = "Test Author";
    
    graph_.SetProjectName(project_name);
    graph_.SetAuthor(author);
    
    // Serialize the graph
    nlohmann::json json = graph_.Serialize();
    
    // Verify all metadata fields are present and correct
    EXPECT_EQ(json["metadata"]["project_name"].get<std::string>(), project_name);
    EXPECT_EQ(json["metadata"]["version"].get<std::string>(), "1.0");
    EXPECT_EQ(json["metadata"]["author"].get<std::string>(), author);
    
    // Verify timestamps are present and are strings
    EXPECT_TRUE(json["metadata"]["created_at"].is_string());
    EXPECT_TRUE(json["metadata"]["modified_at"].is_string());
    
    // Verify timestamp strings are not empty
    EXPECT_FALSE(json["metadata"]["created_at"].get<std::string>().empty());
    EXPECT_FALSE(json["metadata"]["modified_at"].get<std::string>().empty());
}

TEST_F(GraphTest, SerializeIncludesNextId) {
    // Add multiple nodes to increment the ID counter
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    graph_.AddNode(core::NodeBase::NodeKind::kFunction, {0, 0});
    
    // Serialize the graph
    nlohmann::json json = graph_.Serialize();
    
    // Verify next_id is present
    EXPECT_TRUE(json["graph"].contains("next_id"));
    EXPECT_TRUE(json["graph"]["next_id"].is_number_unsigned());
    
    // Next ID should be 3 (after IDs 0, 1, 2 have been used)
    EXPECT_EQ(json["graph"]["next_id"].get<uint32_t>(), 3);
}

TEST_F(GraphTest, SerializeIncludesAllNodes) {
    // Add multiple nodes of different types
    auto *literal_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *variable_node = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *function_node = graph_.AddNode(core::NodeBase::NodeKind::kFunction, {0, 0});
    
    ASSERT_NE(literal_node, nullptr);
    ASSERT_NE(variable_node, nullptr);
    ASSERT_NE(function_node, nullptr);
    
    // Serialize the graph
    nlohmann::json json = graph_.Serialize();
    
    // Verify nodes array contains all nodes
    EXPECT_TRUE(json["graph"]["nodes"].is_array());
    EXPECT_EQ(json["graph"]["nodes"].size(), 3);
    
    // Verify each node has required fields
    for (const auto &node_json : json["graph"]["nodes"]) {
        EXPECT_TRUE(node_json.contains("id"));
        EXPECT_TRUE(node_json.contains("kind"));
        EXPECT_TRUE(node_json["id"].is_number_unsigned());
        EXPECT_TRUE(node_json["kind"].is_string());
    }
    
    // Verify node IDs are present
    std::vector<uint32_t> ids;
    for (const auto &node_json : json["graph"]["nodes"]) {
        ids.push_back(node_json["id"].get<uint32_t>());
    }
    
    EXPECT_NE(std::find(ids.begin(), ids.end(), literal_node->id()), ids.end());
    EXPECT_NE(std::find(ids.begin(), ids.end(), variable_node->id()), ids.end());
    EXPECT_NE(std::find(ids.begin(), ids.end(), function_node->id()), ids.end());
}

TEST_F(GraphTest, SerializeIncludesAllConnections) {
    // Create a small graph with connections
    auto *node1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *node2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *node3 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    ASSERT_NE(node3, nullptr);
    
    // Create connections: node1 -> node2, node1 -> node3
    graph_.Link(node1, 0, node2, 0);
    graph_.Link(node1, 0, node3, 0);
    
    // Serialize the graph
    nlohmann::json json = graph_.Serialize();
    
    // Verify connections array
    EXPECT_TRUE(json["graph"]["connections"].is_array());
    EXPECT_EQ(json["graph"]["connections"].size(), 2);
    
    // Verify each connection has required fields
    for (const auto &conn_json : json["graph"]["connections"]) {
        EXPECT_TRUE(conn_json.contains("source_node_id"));
        EXPECT_TRUE(conn_json.contains("source_pin"));
        EXPECT_TRUE(conn_json.contains("target_node_id"));
        EXPECT_TRUE(conn_json.contains("target_pin"));
        
        EXPECT_TRUE(conn_json["source_node_id"].is_number_unsigned());
        EXPECT_TRUE(conn_json["source_pin"].is_number_unsigned());
        EXPECT_TRUE(conn_json["target_node_id"].is_number_unsigned());
        EXPECT_TRUE(conn_json["target_pin"].is_number_unsigned());
    }
    
    // Verify the connections are correct
    int connections_found = 0;
    for (const auto &conn_json : json["graph"]["connections"]) {
        uint32_t source_id = conn_json["source_node_id"].get<uint32_t>();
        uint32_t target_id = conn_json["target_node_id"].get<uint32_t>();
        
        if (source_id == node1->id() && target_id == node2->id()) {
            connections_found++;
        }
        if (source_id == node1->id() && target_id == node3->id()) {
            connections_found++;
        }
    }
    
    EXPECT_EQ(connections_found, 2);
}

TEST_F(GraphTest, SerializeTimestampsInISO8601Format) {
    // Serialize the graph
    nlohmann::json json = graph_.Serialize();
    
    // Get timestamp strings
    std::string created_at = json["metadata"]["created_at"].get<std::string>();
    std::string modified_at = json["metadata"]["modified_at"].get<std::string>();
    
    // ISO 8601 format: YYYY-MM-DDTHH:MM:SSZ
    // Basic validation of format (20 characters, proper separators)
    EXPECT_EQ(created_at.length(), 20);
    EXPECT_EQ(modified_at.length(), 20);
    
    // Verify format structure
    EXPECT_EQ(created_at[4], '-');   // Year-Month separator
    EXPECT_EQ(created_at[7], '-');   // Month-Day separator
    EXPECT_EQ(created_at[10], 'T');  // Date-Time separator
    EXPECT_EQ(created_at[13], ':');  // Hour-Minute separator
    EXPECT_EQ(created_at[16], ':');  // Minute-Second separator
    EXPECT_EQ(created_at[19], 'Z');  // UTC timezone marker
    
    EXPECT_EQ(modified_at[4], '-');
    EXPECT_EQ(modified_at[7], '-');
    EXPECT_EQ(modified_at[10], 'T');
    EXPECT_EQ(modified_at[13], ':');
    EXPECT_EQ(modified_at[16], ':');
    EXPECT_EQ(modified_at[19], 'Z');
    
    // Verify that all other characters are digits (except separators and 'T', 'Z')
    auto is_valid_char = [](char c, size_t pos) {
        if (pos == 4 || pos == 7 || pos == 13 || pos == 16) return c == '-' || c == ':';
        if (pos == 10) return c == 'T';
        if (pos == 19) return c == 'Z';
        return static_cast<bool>(std::isdigit(c));
    };
    
    for (size_t i = 0; i < created_at.length(); ++i) {
        EXPECT_TRUE(is_valid_char(created_at[i], i)) 
            << "Invalid character at position " << i << " in created_at: " << created_at;
    }
    
    for (size_t i = 0; i < modified_at.length(); ++i) {
        EXPECT_TRUE(is_valid_char(modified_at[i], i))
            << "Invalid character at position " << i << " in modified_at: " << modified_at;
    }
}

#pragma endregion Serialization
/*
#pragma region Deserialization

TEST_F(GraphTest, DeserializeRestoresEmptyGraph) {
    // Set some metadata
    graph_.SetProjectName("Empty Graph Test");
    graph_.SetAuthor("Test Author");
    
    // Serialize the empty graph
    nlohmann::json json = graph_.Serialize();
    
    // Deserialize it back
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_TRUE(result.has_value());
    
    const auto &restored_graph = result.value();
    
    // Verify metadata was restored
    EXPECT_EQ(restored_graph.GetProjectName(), "Empty Graph Test");
    EXPECT_EQ(restored_graph.GetAuthor(), "Test Author");
    EXPECT_EQ(restored_graph.GetVersion(), "1.0");
    
    // Verify graph is empty (no nodes)
    EXPECT_EQ(restored_graph.GetNode(0), nullptr);
}

TEST_F(GraphTest, DeserializeRestoresGraphWithNodes) {
    // Create a graph with multiple nodes
    auto *literal = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *variable = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *function = graph_.AddNode(core::NodeBase::NodeKind::kFunction, {0, 0});
    
    ASSERT_NE(literal, nullptr);
    ASSERT_NE(variable, nullptr);
    ASSERT_NE(function, nullptr);
    
    uint32_t literal_id = literal->id();
    uint32_t variable_id = variable->id();
    uint32_t function_id = function->id();
    
    // Serialize the graph
    nlohmann::json json = graph_.Serialize();
    
    // Deserialize it back
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_TRUE(result.has_value());
    
    auto &restored_graph = result.value();
    
    // Verify all nodes were restored
    auto *restored_literal = restored_graph.GetNode(literal_id);
    auto *restored_variable = restored_graph.GetNode(variable_id);
    auto *restored_function = restored_graph.GetNode(function_id);
    
    ASSERT_NE(restored_literal, nullptr);
    ASSERT_NE(restored_variable, nullptr);
    ASSERT_NE(restored_function, nullptr);
    
    // Verify node kinds
    EXPECT_EQ(restored_literal->kind(), core::NodeBase::NodeKind::kLiteral);
    EXPECT_EQ(restored_variable->kind(), core::NodeBase::NodeKind::kVariable);
    EXPECT_EQ(restored_function->kind(), core::NodeBase::NodeKind::kFunction);
}

TEST_F(GraphTest, DeserializeRestoresGraphWithConnections) {
    // Create a graph with connections
    auto *node1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *node2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *node3 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    ASSERT_NE(node3, nullptr);
    
    uint32_t id1 = node1->id();
    uint32_t id2 = node2->id();
    uint32_t id3 = node3->id();
    
    // Create connections: node1 -> node2, node2 -> node3
    graph_.Link(node1, 0, node2, 0);
    graph_.Link(node2, 0, node3, 0);
    
    // Serialize the graph
    nlohmann::json json = graph_.Serialize();
    
    // Deserialize it back
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_TRUE(result.has_value());
    
    auto &restored_graph = result.value();
    
    // Get restored nodes
    auto *restored1 = restored_graph.GetNode(id1);
    auto *restored2 = restored_graph.GetNode(id2);
    auto *restored3 = restored_graph.GetNode(id3);
    
    ASSERT_NE(restored1, nullptr);
    ASSERT_NE(restored2, nullptr);
    ASSERT_NE(restored3, nullptr);
    
    // Verify connections were restored
    EXPECT_TRUE(restored2->IsInputPinConnected(0));
    EXPECT_TRUE(restored3->IsInputPinConnected(0));
    
    // Verify parent connections
    auto parent2 = restored2->parent(0);
    EXPECT_EQ(parent2.node->id(), id1);
    
    auto parent3 = restored3->parent(0);
    EXPECT_EQ(parent3.node->id(), id2);
}

TEST_F(GraphTest, DeserializeRestoresMetadata) {
    // Set specific metadata
    const std::string project_name = "Metadata Restore Test";
    const std::string author = "Deserialization Tester";
    
    graph_.SetProjectName(project_name);
    graph_.SetAuthor(author);
    
    auto original_created = graph_.GetCreatedAt();
    auto original_modified = graph_.GetModifiedAt();
    
    // Serialize the graph
    nlohmann::json json = graph_.Serialize();
    
    // Deserialize it back
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_TRUE(result.has_value());
    
    const auto &restored_graph = result.value();
    
    // Verify all metadata fields
    EXPECT_EQ(restored_graph.GetProjectName(), project_name);
    EXPECT_EQ(restored_graph.GetVersion(), "1.0");
    EXPECT_EQ(restored_graph.GetAuthor(), author);
    
    // Verify timestamps (converted to time_t for comparison to avoid precision issues)
    auto restored_created = restored_graph.GetCreatedAt();
    auto restored_modified = restored_graph.GetModifiedAt();
    
    auto original_created_t = std::chrono::system_clock::to_time_t(original_created);
    auto restored_created_t = std::chrono::system_clock::to_time_t(restored_created);
    
    auto original_modified_t = std::chrono::system_clock::to_time_t(original_modified);
    auto restored_modified_t = std::chrono::system_clock::to_time_t(restored_modified);
    
    EXPECT_EQ(original_created_t, restored_created_t);
    EXPECT_EQ(original_modified_t, restored_modified_t);
}

TEST_F(GraphTest, DeserializeRestoresNextId) {
    // Add some nodes to increment the ID counter
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    graph_.AddNode(core::NodeBase::NodeKind::kFunction, {0, 0});
    
    // Serialize the graph
    nlohmann::json json = graph_.Serialize();
    
    // Verify next_id in JSON
    EXPECT_EQ(json["graph"]["next_id"].get<uint32_t>(), 3);
    
    // Deserialize it back
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_TRUE(result.has_value());
    
    auto &restored_graph = result.value();
    
    // Add a new node to verify the ID counter was restored
    auto *new_node = restored_graph.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    ASSERT_NE(new_node, nullptr);
    
    // The new node should get ID 3 (next available after 0, 1, 2)
    EXPECT_EQ(new_node->id(), 3);
}

TEST_F(GraphTest, DeserializeFailsOnMissingMetadata) {
    nlohmann::json json;
    
    // Create JSON with missing metadata section
    json["graph"]["next_id"] = 0;
    json["graph"]["nodes"] = nlohmann::json::array();
    json["graph"]["connections"] = nlohmann::json::array();
    
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("metadata"), std::string::npos);
}

TEST_F(GraphTest, DeserializeFailsOnMissingGraph) {
    nlohmann::json json;
    
    // Create JSON with missing graph section
    json["metadata"]["project_name"] = "Test";
    json["metadata"]["version"] = "1.0";
    json["metadata"]["author"] = "Test";
    json["metadata"]["created_at"] = "2026-01-01T00:00:00Z";
    json["metadata"]["modified_at"] = "2026-01-01T00:00:00Z";
    
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("graph"), std::string::npos);
}

TEST_F(GraphTest, DeserializeFailsOnInvalidVersion) {
    // Create a valid serialized graph
    nlohmann::json json = graph_.Serialize();
    
    // Change version to unsupported value
    json["metadata"]["version"] = "2.0";
    
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("version"), std::string::npos);
}

TEST_F(GraphTest, DeserializeFailsOnMalformedNodes) {
    nlohmann::json json = graph_.Serialize();
    
    // Make nodes not an array
    json["graph"]["nodes"] = "not an array";
    
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("nodes"), std::string::npos);
}

TEST_F(GraphTest, DeserializeFailsOnMalformedConnections) {
    nlohmann::json json = graph_.Serialize();
    
    // Make connections not an array
    json["graph"]["connections"] = "not an array";
    
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("connections"), std::string::npos);
}

TEST_F(GraphTest, DeserializeFailsOnNonexistentConnectionNode) {
    // Create a graph with a node
    auto *node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    ASSERT_NE(node, nullptr);
    
    // Serialize the graph
    nlohmann::json json = graph_.Serialize();
    
    // Add a connection referencing a non-existent node
    nlohmann::json fake_connection;
    fake_connection["source_node_id"] = 999;
    fake_connection["source_pin"] = 0;
    fake_connection["target_node_id"] = node->id();
    fake_connection["target_pin"] = 0;
    
    json["graph"]["connections"].push_back(fake_connection);
    
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("non-existent"), std::string::npos);
}

TEST_F(GraphTest, DeserializeFailsOnInvalidPinIndex) {
    // Create two nodes
    auto *node1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *node2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    
    // Serialize the graph
    nlohmann::json json = graph_.Serialize();
    
    // Add a connection with invalid pin index
    nlohmann::json bad_connection;
    bad_connection["source_node_id"] = node1->id();
    bad_connection["source_pin"] = 255;  // Invalid pin index
    bad_connection["target_node_id"] = node2->id();
    bad_connection["target_pin"] = 0;
    
    json["graph"]["connections"].push_back(bad_connection);
    
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("pin"), std::string::npos);
}

TEST_F(GraphTest, DeserializeFailsOnIncompatiblePinTypes) {
    // Create two nodes with incompatible types
    auto *literal = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto *variable = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable);
    
    ASSERT_NE(literal, nullptr);
    ASSERT_NE(variable, nullptr);
    
    // Set incompatible types
    literal->set_data(42.0f);  // Float
    variable->SetType(core::NodeBase::PinDataType::kInt);  // Int
    
    // Serialize the graph
    nlohmann::json json = graph_.Serialize();
    
    // Manually add a connection with incompatible types
    nlohmann::json bad_connection;
    bad_connection["source_node_id"] = literal->id();
    bad_connection["source_pin"] = 0;
    bad_connection["target_node_id"] = variable->id();
    bad_connection["target_pin"] = 0;
    
    json["graph"]["connections"].push_back(bad_connection);
    
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("cannot be connected"), std::string::npos);
}

#pragma endregion Deserialization

#pragma region File I/O

TEST_F(GraphTest, SaveToFileCreatesFile) {
    // Create a test file path
    std::filesystem::path test_path = std::filesystem::temp_directory_path() / "test_graph.nebula";
    
    // Ensure the file doesn't exist before the test
    std::filesystem::remove(test_path);
    
    // Add some nodes and connections
    auto *node1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *node2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    
    graph_.Link(node1, 0, node2, 0);
    
    // Save the graph
    auto result = graph_.SaveToFile(test_path);
    
    ASSERT_TRUE(result.has_value()) << "SaveToFile failed: " << result.error();
    
    // Verify file exists
    EXPECT_TRUE(std::filesystem::exists(test_path));
    
    // Verify file contains valid JSON
    std::ifstream file(test_path);
    ASSERT_TRUE(file.is_open());
    
    nlohmann::json json;
    EXPECT_NO_THROW({
        file >> json;
    });
    
    file.close();
    
    // Verify JSON structure
    EXPECT_TRUE(json.contains("metadata"));
    EXPECT_TRUE(json.contains("graph"));
    
    // Clean up
    std::filesystem::remove(test_path);
}

TEST_F(GraphTest, SaveToFileUpdatesModifiedTime) {
    std::filesystem::path test_path = std::filesystem::temp_directory_path() / "test_modified_time.nebula";
    
    auto initial_modified = graph_.GetModifiedAt();
    
    // Sleep to ensure time difference
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Save the graph
    auto result = graph_.SaveToFile(test_path);
    
    ASSERT_TRUE(result.has_value());
    
    auto new_modified = graph_.GetModifiedAt();
    
    // Verify modified time was updated
    EXPECT_GT(new_modified, initial_modified);
    
    // Clean up
    std::filesystem::remove(test_path);
}

TEST_F(GraphTest, SaveToFilePrettyPrintsJson) {
    std::filesystem::path test_path = std::filesystem::temp_directory_path() / "test_pretty_print.nebula";
    
    // Add a node
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    
    // Save the graph
    auto result = graph_.SaveToFile(test_path);
    
    ASSERT_TRUE(result.has_value());
    
    // Read the file as text
    std::ifstream file(test_path);
    ASSERT_TRUE(file.is_open());
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    // Verify pretty printing (indentation)
    // Pretty printed JSON should contain newlines and spaces
    EXPECT_NE(content.find('\n'), std::string::npos);
    EXPECT_NE(content.find("  "), std::string::npos);  // Indentation (4 spaces)
    
    // Clean up
    std::filesystem::remove(test_path);
}

TEST_F(GraphTest, SaveToFileFailsOnInvalidPath) {
    // Try to save to an invalid/inaccessible path
    std::filesystem::path invalid_path = "/root/nonexistent/directory/file.nebula";
    
    auto result = graph_.SaveToFile(invalid_path);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_FALSE(result.error().empty());
}

TEST_F(GraphTest, LoadFromFileRestoresGraph) {
    std::filesystem::path test_path = std::filesystem::temp_directory_path() / "test_load.nebula";
    
    // Create and save a graph
    graph_.SetProjectName("Load Test Graph");
    graph_.SetAuthor("Test Author");
    
    auto *node1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *node2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    
    uint32_t id1 = node1->id();
    uint32_t id2 = node2->id();
    
    graph_.Link(node1, 0, node2, 0);
    
    auto save_result = graph_.SaveToFile(test_path);
    ASSERT_TRUE(save_result.has_value());
    
    // Load the graph
    auto load_result = core::Graph::LoadFromFile(test_path);
    
    ASSERT_TRUE(load_result.has_value()) << "LoadFromFile failed: " << load_result.error();
    
    const auto &loaded_graph = load_result.value();
    
    // Verify metadata
    EXPECT_EQ(loaded_graph.GetProjectName(), "Load Test Graph");
    EXPECT_EQ(loaded_graph.GetAuthor(), "Test Author");
    
    // Verify nodes
    auto *loaded_node1 = loaded_graph.GetNode(id1);
    auto *loaded_node2 = loaded_graph.GetNode(id2);
    
    ASSERT_NE(loaded_node1, nullptr);
    ASSERT_NE(loaded_node2, nullptr);
    
    // Verify connection
    EXPECT_TRUE(loaded_node2->IsInputPinConnected(0));
    
    // Clean up
    std::filesystem::remove(test_path);
}

TEST_F(GraphTest, LoadFromFileFailsOnNonexistentFile) {
    std::filesystem::path nonexistent_path = std::filesystem::temp_directory_path() / "nonexistent_file.nebula";
    
    // Ensure file doesn't exist
    std::filesystem::remove(nonexistent_path);
    
    auto result = core::Graph::LoadFromFile(nonexistent_path);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("does not exist"), std::string::npos);
}

TEST_F(GraphTest, LoadFromFileFailsOnInvalidJson) {
    std::filesystem::path test_path = std::filesystem::temp_directory_path() / "invalid_json.nebula";
    
    // Create a file with invalid JSON
    std::ofstream file(test_path);
    ASSERT_TRUE(file.is_open());
    file << "{ this is not valid json }";
    file.close();
    
    auto result = core::Graph::LoadFromFile(test_path);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("parse"), std::string::npos);
    
    // Clean up
    std::filesystem::remove(test_path);
}

TEST_F(GraphTest, LoadFromFileFailsOnCorruptedData) {
    std::filesystem::path test_path = std::filesystem::temp_directory_path() / "corrupted_data.nebula";
    
    // Create a file with valid JSON but corrupted structure
    std::ofstream file(test_path);
    ASSERT_TRUE(file.is_open());
    
    nlohmann::json corrupted_json;
    corrupted_json["metadata"]["project_name"] = "Test";
    corrupted_json["metadata"]["version"] = "1.0";
    // Missing other required fields
    
    file << corrupted_json.dump(4);
    file.close();
    
    auto result = core::Graph::LoadFromFile(test_path);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_FALSE(result.error().empty());
    
    // Clean up
    std::filesystem::remove(test_path);
}

#pragma endregion File I/O

#pragma region Integration

TEST_F(GraphTest, RoundTripSerializationPreservesGraph) {
    // Create a complex graph
    graph_.SetProjectName("Round Trip Test");
    graph_.SetAuthor("Integration Tester");
    
    auto *literal = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *variable1 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *variable2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *function = graph_.AddNode(core::NodeBase::NodeKind::kFunction, {0, 0});
    
    ASSERT_NE(literal, nullptr);
    ASSERT_NE(variable1, nullptr);
    ASSERT_NE(variable2, nullptr);
    ASSERT_NE(function, nullptr);
    
    uint32_t literal_id = literal->id();
    uint32_t var1_id = variable1->id();
    uint32_t var2_id = variable2->id();
    uint32_t func_id = function->id();
    
    // Create connections
    graph_.Link(literal, 0, variable1, 0);
    graph_.Link(variable1, 0, variable2, 0);
    
    // Serialize
    nlohmann::json json = graph_.Serialize();
    
    // Deserialize
    auto result = core::Graph::Deserialize(json);
    
    ASSERT_TRUE(result.has_value());
    
    auto &restored_graph = result.value();
    
    // Verify metadata
    EXPECT_EQ(restored_graph.GetProjectName(), "Round Trip Test");
    EXPECT_EQ(restored_graph.GetAuthor(), "Integration Tester");
    EXPECT_EQ(restored_graph.GetVersion(), "1.0");
    
    // Verify all nodes exist
    EXPECT_NE(restored_graph.GetNode(literal_id), nullptr);
    EXPECT_NE(restored_graph.GetNode(var1_id), nullptr);
    EXPECT_NE(restored_graph.GetNode(var2_id), nullptr);
    EXPECT_NE(restored_graph.GetNode(func_id), nullptr);
    
    // Verify connections
    auto *restored_var1 = restored_graph.GetNode(var1_id);
    auto *restored_var2 = restored_graph.GetNode(var2_id);
    
    EXPECT_TRUE(restored_var1->IsInputPinConnected(0));
    EXPECT_TRUE(restored_var2->IsInputPinConnected(0));
    
    auto parent1 = restored_var1->parent(0);
    EXPECT_EQ(parent1.node->id(), literal_id);
    
    auto parent2 = restored_var2->parent(0);
    EXPECT_EQ(parent2.node->id(), var1_id);
}

TEST_F(GraphTest, RoundTripFileIOPreservesGraph) {
    std::filesystem::path test_path = std::filesystem::temp_directory_path() / "round_trip_file.nebula";
    
    // Create a graph
    graph_.SetProjectName("File Round Trip Test");
    graph_.SetAuthor("File Tester");
    
    auto *node1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *node2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *node3 = graph_.AddNode(core::NodeBase::NodeKind::kFunction, {0, 0});
    
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    ASSERT_NE(node3, nullptr);
    
    uint32_t id1 = node1->id();
    uint32_t id2 = node2->id();
    uint32_t id3 = node3->id();
    
    graph_.Link(node1, 0, node2, 0);
    
    // Save to file
    auto save_result = graph_.SaveToFile(test_path);
    ASSERT_TRUE(save_result.has_value());
    
    // Load from file
    auto load_result = core::Graph::LoadFromFile(test_path);
    ASSERT_TRUE(load_result.has_value());
    
    const auto &loaded_graph = load_result.value();
    
    // Verify metadata
    EXPECT_EQ(loaded_graph.GetProjectName(), "File Round Trip Test");
    EXPECT_EQ(loaded_graph.GetAuthor(), "File Tester");
    
    // Verify nodes
    EXPECT_NE(loaded_graph.GetNode(id1), nullptr);
    EXPECT_NE(loaded_graph.GetNode(id2), nullptr);
    EXPECT_NE(loaded_graph.GetNode(id3), nullptr);
    
    // Verify connection
    auto *loaded_node2 = loaded_graph.GetNode(id2);
    ASSERT_NE(loaded_node2, nullptr);
    EXPECT_TRUE(loaded_node2->IsInputPinConnected(0));
    
    auto parent = loaded_node2->parent(0);
    EXPECT_EQ(parent.node->id(), id1);
    
    // Clean up
    std::filesystem::remove(test_path);
}

TEST_F(GraphTest, ComplexGraphWithMultipleConnectionsSerialization) {
    // Create a more complex graph with multiple connections
    auto *lit1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *lit2 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *var1 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *var2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *var3 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *func = graph_.AddNode(core::NodeBase::NodeKind::kFunction, {0, 0});
    
    ASSERT_NE(lit1, nullptr);
    ASSERT_NE(lit2, nullptr);
    ASSERT_NE(var1, nullptr);
    ASSERT_NE(var2, nullptr);
    ASSERT_NE(var3, nullptr);
    ASSERT_NE(func, nullptr);
    
    // Create a complex connection pattern
    // lit1 -> var1, lit2 -> var2
    // var1 -> var3, var2 -> var3
    graph_.Link(lit1, 0, var1, 0);
    graph_.Link(lit2, 0, var2, 0);
    graph_.Link(var1, 0, var3, 0);
    
    // Serialize
    nlohmann::json json = graph_.Serialize();
    
    // Verify all connections are serialized
    EXPECT_EQ(json["graph"]["connections"].size(), 3);
    
    // Deserialize
    auto result = core::Graph::Deserialize(json);
    ASSERT_TRUE(result.has_value());
    
    auto &restored = result.value();
    
    // Verify all nodes exist
    EXPECT_EQ(restored.GetNode(0), nullptr);  // First node deleted, but IDs 0-5 should exist
    EXPECT_NE(restored.GetNode(lit1->id()), nullptr);
    EXPECT_NE(restored.GetNode(lit2->id()), nullptr);
    EXPECT_NE(restored.GetNode(var1->id()), nullptr);
    EXPECT_NE(restored.GetNode(var2->id()), nullptr);
    EXPECT_NE(restored.GetNode(var3->id()), nullptr);
    EXPECT_NE(restored.GetNode(func->id()), nullptr);
    
    // Verify connections
    auto *restored_var1 = restored.GetNode(var1->id());
    auto *restored_var2 = restored.GetNode(var2->id());
    auto *restored_var3 = restored.GetNode(var3->id());
    
    EXPECT_TRUE(restored_var1->IsInputPinConnected(0));
    EXPECT_TRUE(restored_var2->IsInputPinConnected(0));
    EXPECT_TRUE(restored_var3->IsInputPinConnected(0));
}

TEST_F(GraphTest, RemoveNodeFromConnectedGraphMaintainsIntegrity) {
    // Create a chain of 5 nodes
    auto *node1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *node2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *node3 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *node4 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *node5 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {0, 0});
    
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    ASSERT_NE(node3, nullptr);
    ASSERT_NE(node4, nullptr);
    ASSERT_NE(node5, nullptr);
    
    // uint32_t id1 = node1->id();
    uint32_t id2 = node2->id();
    uint32_t id3 = node3->id();
    uint32_t id4 = node4->id();
    // uint32_t id5 = node5->id();
    
    // Create chain: 1 -> 2 -> 3 -> 4 -> 5
    graph_.Link(node1, 0, node2, 0);
    graph_.Link(node2, 0, node3, 0);
    graph_.Link(node3, 0, node4, 0);
    graph_.Link(node4, 0, node5, 0);
    
    // Verify all connections exist
    EXPECT_TRUE(node2->IsInputPinConnected(0));
    EXPECT_TRUE(node3->IsInputPinConnected(0));
    EXPECT_TRUE(node4->IsInputPinConnected(0));
    EXPECT_TRUE(node5->IsInputPinConnected(0));
    
    // Remove middle node (node3)
    graph_.RemoveNode(node3);
    
    // Verify node3 is gone
    EXPECT_EQ(graph_.GetNode(id3), nullptr);
    
    // Verify node2 and node4 still exist
    EXPECT_NE(graph_.GetNode(id2), nullptr);
    EXPECT_NE(graph_.GetNode(id4), nullptr);
    
    // Verify node2's output connection was cleared
    EXPECT_FALSE(node2->IsOutputPinConnected(0));
    
    // Verify node4's input connection was cleared
    EXPECT_FALSE(node4->IsInputPinConnected(0));
    
    // Verify other connections remain intact
    EXPECT_TRUE(node2->IsInputPinConnected(0));  // 1 -> 2 still connected
    EXPECT_TRUE(node5->IsInputPinConnected(0));  // 4 -> 5 still connected
    
    // Serialize to verify graph is still valid
    nlohmann::json json = graph_.Serialize();
    
    // Should have 3 connections remaining (1->2, 4->5, and any others)
    // Actually we have: 1->2 and 4->5, so 2 connections
    EXPECT_EQ(json["graph"]["connections"].size(), 2);
    
    // Deserialize to verify integrity
    auto result = core::Graph::Deserialize(json);
    ASSERT_TRUE(result.has_value());
}

#pragma endregion Integration
*/