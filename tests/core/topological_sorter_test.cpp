#include <gtest/gtest.h>
#include <unordered_set>

#include "core/graph.hpp"
#include "core/topological_sorter.hpp"
#include "core/nodes/literal_node.hpp"
#include "core/nodes/operator_node.hpp"
#include "core/nodes/variable_node.hpp"

class TopologicalSorterTest : public ::testing::Test {
   protected:
    core::Graph graph_;
    core::TopologicalSorter sorter_;
};

// ============================================================================
// Basic Sorting Tests
// ============================================================================

TEST_F(TopologicalSorterTest, EmptyGraphReturnsEmptyList) {
    auto sorted = core::TopologicalSorter::Sort(graph_);
    EXPECT_EQ(sorted.size(), 0);
}

TEST_F(TopologicalSorterTest, SingleNodeReturnsOneNode) {
    auto *lit = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    ASSERT_NE(lit, nullptr);
    
    auto sorted = core::TopologicalSorter::Sort(graph_);
    EXPECT_EQ(sorted.size(), 1);
    EXPECT_EQ(sorted[0]->id(), lit->id());
}

TEST_F(TopologicalSorterTest, LinearChainIsSorted) {
    // Create: Lit → Op1 → Op2 → Op3
    auto *lit = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *op1 = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    auto *op2 = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    auto *op3 = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    
    ASSERT_NE(lit, nullptr);
    ASSERT_NE(op1, nullptr);
    ASSERT_NE(op2, nullptr);
    ASSERT_NE(op3, nullptr);
    
    lit->SetType(core::NodeBase::PinDataType::kInt);
    op1->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    op2->SetOperatorType(core::OperatorNode::OperatorType::kMultiplication);
    op3->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    
    // Connect the chain
    graph_.Link(lit, 0, op1, 0);
    graph_.Link(lit, 0, op1, 1);
    graph_.Link(op1, 0, op2, 0);
    graph_.Link(op1, 0, op2, 1);
    graph_.Link(op2, 0, op3, 0);
    graph_.Link(op2, 0, op3, 1);
    
    auto sorted = core::TopologicalSorter::Sort(graph_);
    
    // Should have all 4 nodes
    EXPECT_EQ(sorted.size(), 4);
    
    // Verify ordering: lit should come before op1, op1 before op2, op2 before op3
    auto lit_idx = std::find_if(sorted.begin(), sorted.end(),
                                [lit](const auto* n) { return n->id() == lit->id(); }) - sorted.begin();
    auto op1_idx = std::find_if(sorted.begin(), sorted.end(),
                                [op1](const auto* n) { return n->id() == op1->id(); }) - sorted.begin();
    auto op2_idx = std::find_if(sorted.begin(), sorted.end(),
                                [op2](const auto* n) { return n->id() == op2->id(); }) - sorted.begin();
    auto op3_idx = std::find_if(sorted.begin(), sorted.end(),
                                [op3](const auto* n) { return n->id() == op3->id(); }) - sorted.begin();
    
    EXPECT_LT(lit_idx, op1_idx);
    EXPECT_LT(op1_idx, op2_idx);
    EXPECT_LT(op2_idx, op3_idx);
}

// ============================================================================
// Branching and Merging Tests
// ============================================================================

TEST_F(TopologicalSorterTest, DiamondGraphIsTopologicallySorted) {
    // Create diamond: Lit → Op1, Lit → Op2, Op1 → Op3, Op2 → Op3
    auto *lit = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *op1 = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    auto *op2 = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    auto *op3 = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    
    ASSERT_NE(lit, nullptr);
    ASSERT_NE(op1, nullptr);
    ASSERT_NE(op2, nullptr);
    ASSERT_NE(op3, nullptr);
    
    lit->SetType(core::NodeBase::PinDataType::kInt);
    op1->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    op2->SetOperatorType(core::OperatorNode::OperatorType::kMultiplication);
    op3->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    
    // Create diamond connections
    graph_.Link(lit, 0, op1, 0);
    graph_.Link(lit, 0, op1, 1);
    graph_.Link(lit, 0, op2, 0);
    graph_.Link(lit, 0, op2, 1);
    graph_.Link(op1, 0, op3, 0);
    graph_.Link(op2, 0, op3, 1);
    
    auto sorted = core::TopologicalSorter::Sort(graph_);
    
    EXPECT_EQ(sorted.size(), 4);
    
    // Verify ordering
    auto lit_idx = std::find_if(sorted.begin(), sorted.end(),
                                [lit](const auto* n) { return n->id() == lit->id(); }) - sorted.begin();
    auto op1_idx = std::find_if(sorted.begin(), sorted.end(),
                                [op1](const auto* n) { return n->id() == op1->id(); }) - sorted.begin();
    auto op2_idx = std::find_if(sorted.begin(), sorted.end(),
                                [op2](const auto* n) { return n->id() == op2->id(); }) - sorted.begin();
    auto op3_idx = std::find_if(sorted.begin(), sorted.end(),
                                [op3](const auto* n) { return n->id() == op3->id(); }) - sorted.begin();
    
    EXPECT_LT(lit_idx, op1_idx);
    EXPECT_LT(lit_idx, op2_idx);
    EXPECT_LT(op1_idx, op3_idx);
    EXPECT_LT(op2_idx, op3_idx);
}

TEST_F(TopologicalSorterTest, MultipleIndependentPathsAreSorted) {
    // Create two independent paths: Lit1→Op1 and Lit2→Op2
    auto *lit1 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *lit2 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *op1 = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    auto *op2 = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    
    ASSERT_NE(lit1, nullptr);
    ASSERT_NE(lit2, nullptr);
    ASSERT_NE(op1, nullptr);
    ASSERT_NE(op2, nullptr);
    
    lit1->SetType(core::NodeBase::PinDataType::kInt);
    lit2->SetType(core::NodeBase::PinDataType::kInt);
    op1->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    op2->SetOperatorType(core::OperatorNode::OperatorType::kMultiplication);
    
    // Create two independent paths
    graph_.Link(lit1, 0, op1, 0);
    graph_.Link(lit1, 0, op1, 1);
    graph_.Link(lit2, 0, op2, 0);
    graph_.Link(lit2, 0, op2, 1);
    
    auto sorted = core::TopologicalSorter::Sort(graph_);
    
    EXPECT_EQ(sorted.size(), 4);
    
    // Verify ordering constraints
    auto lit1_idx = std::find_if(sorted.begin(), sorted.end(),
                                 [lit1](const auto* n) { return n->id() == lit1->id(); }) - sorted.begin();
    auto lit2_idx = std::find_if(sorted.begin(), sorted.end(),
                                 [lit2](const auto* n) { return n->id() == lit2->id(); }) - sorted.begin();
    auto op1_idx = std::find_if(sorted.begin(), sorted.end(),
                                [op1](const auto* n) { return n->id() == op1->id(); }) - sorted.begin();
    auto op2_idx = std::find_if(sorted.begin(), sorted.end(),
                                [op2](const auto* n) { return n->id() == op2->id(); }) - sorted.begin();
    
    EXPECT_LT(lit1_idx, op1_idx);
    EXPECT_LT(lit2_idx, op2_idx);
}

// ============================================================================
// Large Graph Tests
// ============================================================================

TEST_F(TopologicalSorterTest, LargeGraphWithManyNodes) {
    // Create 20 literals connected to 15 operators in various ways
    std::vector<core::LiteralNode*> literals;
    std::vector<core::OperatorNode*> operators;
    
    // Create 20 literals
    for (int i = 0; i < 20; ++i) {
        auto *lit = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
        ASSERT_NE(lit, nullptr);
        lit->SetType(core::NodeBase::PinDataType::kInt);
        literals.push_back(lit);
    }
    
    // Create 15 operators
    for (int i = 0; i < 15; ++i) {
        auto *op = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
        ASSERT_NE(op, nullptr);
        op->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
        operators.push_back(op);
    }
    
    // Connect: literals → first layer of operators
    for (int i = 0; i < 10; ++i) {
        graph_.Link(literals[i], 0, operators[0], (i % 2 == 0) ? 0 : 1);
    }
    
    // Connect: chain operators
    for (int i = 1; i < 15; ++i) {
        graph_.Link(operators[i - 1], 0, operators[i], 0);
        if (10 + i < 20) {
            graph_.Link(literals[10 + i], 0, operators[i], 1);
        }
    }
    
    auto sorted = core::TopologicalSorter::Sort(graph_);
    
    // Should have all 35 nodes
    EXPECT_EQ(sorted.size(), 35);
    
    // Verify topological property: if A→B exists, A comes before B
    std::unordered_map<uint32_t, size_t> node_position;
    for (size_t i = 0; i < sorted.size(); ++i) {
        node_position[sorted[i]->id()] = i;
    }
    
    // Check all connections respect topological order
    for (const auto& lit : literals) {
        const auto& children = lit->GetAllChildrens();
        for (const auto& child : children) {
            if (child.IsConnected() && child.node) {
                EXPECT_LT(node_position[lit->id()], node_position[child.node->id()]);
            }
        }
    }
    
    for (const auto& op : operators) {
        const auto& children = op->GetAllChildrens();
        for (const auto& child : children) {
            if (child.IsConnected() && child.node) {
                EXPECT_LT(node_position[op->id()], node_position[child.node->id()]);
            }
        }
    }
}

// ============================================================================
// Complex Topology Tests
// ============================================================================

TEST_F(TopologicalSorterTest, ComplexBranchingAndMergingGraph) {
    // Create a complex graph with multiple paths that branch and merge
    auto *lit1 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *lit2 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *op1 = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    auto *op2 = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    auto *op3 = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    auto *op4 = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    auto *op5 = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    
    ASSERT_NE(lit1, nullptr);
    ASSERT_NE(lit2, nullptr);
    
    lit1->SetType(core::NodeBase::PinDataType::kInt);
    lit2->SetType(core::NodeBase::PinDataType::kInt);
    for (auto op : {op1, op2, op3, op4, op5}) {
        ASSERT_NE(op, nullptr);
        op->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    }
    
    // Build complex graph
    graph_.Link(lit1, 0, op1, 0);
    graph_.Link(lit1, 0, op1, 1);
    graph_.Link(lit2, 0, op2, 0);
    graph_.Link(lit2, 0, op2, 1);
    
    // Branch: op1 → op3, op4
    graph_.Link(op1, 0, op3, 0);
    graph_.Link(op1, 0, op4, 0);
    
    // Branch: op2 → op3, op5
    graph_.Link(op2, 0, op3, 1);
    graph_.Link(op2, 0, op5, 0);
    
    // Merge: op3, op4, op5 → all complete
    graph_.Link(op4, 0, op5, 1);
    
    auto sorted = core::TopologicalSorter::Sort(graph_);
    
    EXPECT_EQ(sorted.size(), 7);
    
    // Verify ordering
    std::unordered_map<uint32_t, size_t> pos;
    for (size_t i = 0; i < sorted.size(); ++i) {
        pos[sorted[i]->id()] = i;
    }
    
    EXPECT_LT(pos[lit1->id()], pos[op1->id()]);
    EXPECT_LT(pos[lit2->id()], pos[op2->id()]);
    EXPECT_LT(pos[op1->id()], pos[op3->id()]);
    EXPECT_LT(pos[op1->id()], pos[op4->id()]);
    EXPECT_LT(pos[op2->id()], pos[op3->id()]);
    EXPECT_LT(pos[op2->id()], pos[op5->id()]);
    EXPECT_LT(pos[op4->id()], pos[op5->id()]);
}

// ============================================================================
// Node Isolation Tests
// ============================================================================

TEST_F(TopologicalSorterTest, DisconnectedNodesAreIncludedInSorting) {
    // Create nodes with no connections between them
    auto *lit1 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *lit2 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *lit3 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    
    ASSERT_NE(lit1, nullptr);
    ASSERT_NE(lit2, nullptr);
    ASSERT_NE(lit3, nullptr);
    
    auto sorted = core::TopologicalSorter::Sort(graph_);
    
    // Should include all 3 nodes
    EXPECT_EQ(sorted.size(), 3);
    
    // Check all nodes are present
    std::unordered_set<uint32_t> sorted_ids;
    for (const auto& node : sorted) {
        sorted_ids.insert(node->id());
    }
    
    EXPECT_EQ(sorted_ids.count(lit1->id()), 1);
    EXPECT_EQ(sorted_ids.count(lit2->id()), 1);
    EXPECT_EQ(sorted_ids.count(lit3->id()), 1);
}

TEST_F(TopologicalSorterTest, AllNodesAppearExactlyOnce) {
    // Verify that topological sort doesn't duplicate nodes
    auto *lit1 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *lit2 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *op = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    
    ASSERT_NE(lit1, nullptr);
    ASSERT_NE(lit2, nullptr);
    ASSERT_NE(op, nullptr);
    
    lit1->SetType(core::NodeBase::PinDataType::kInt);
    lit2->SetType(core::NodeBase::PinDataType::kInt);
    op->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    
    // Both literals connect to operator
    graph_.Link(lit1, 0, op, 0);
    graph_.Link(lit2, 0, op, 1);
    
    auto sorted = core::TopologicalSorter::Sort(graph_);
    
    EXPECT_EQ(sorted.size(), 3);
    
    // Count occurrences
    int lit1_count = 0, lit2_count = 0, op_count = 0;
    for (const auto& node : sorted) {
        if (node->id() == lit1->id()) lit1_count++;
        if (node->id() == lit2->id()) lit2_count++;
        if (node->id() == op->id()) op_count++;
    }
    
    EXPECT_EQ(lit1_count, 1);
    EXPECT_EQ(lit2_count, 1);
    EXPECT_EQ(op_count, 1);
}

