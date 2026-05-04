#include <gtest/gtest.h>

#include "core/graph.hpp"
#include "core/nodes/literal_node.hpp"
#include "core/nodes/operator_node.hpp"
#include "editor/code-generation/codegen-optimizer.hpp"

using namespace core;
using namespace editor::code_generation;

class CodegenOptimizerTest : public ::testing::Test {
   protected:
    Graph graph_;
};

TEST_F(CodegenOptimizerTest, FindUsedNodes_SingleLiteralIsUsed) {
    // Create: lit = 5
    auto *lit = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    lit->set_data(5);
    
    CodegenOptimizer optimizer;
    auto used = optimizer.FindUsedNodes(graph_);
    
    // The literal node is a leaf (no children), so it should be marked as used
    EXPECT_EQ(used.size(), 1);
    EXPECT_TRUE(used.count(lit->id()) > 0);
}

TEST_F(CodegenOptimizerTest, FindUsedNodes_AllNodesContributeToResult) {
    // Create: lit1 = 5, lit2 = 3, add = lit1 + lit2
    auto *lit1 = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    lit1->set_data(5);
    
    auto *lit2 = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    lit2->set_data(3);
    
    auto *add = graph_.AddNode<OperatorNode>(NodeBase::NodeKind::kOperator);
    add->set_operator_type(OperatorNode::OperatorType::kAddition);
    
    graph_.Link(lit1, 0, add, 0);
    graph_.Link(lit2, 0, add, 1);
    
    CodegenOptimizer optimizer;
    auto used = optimizer.FindUsedNodes(graph_);
    
    // All nodes contribute to the result
    EXPECT_EQ(used.size(), 3);
    EXPECT_TRUE(used.count(lit1->id()) > 0);
    EXPECT_TRUE(used.count(lit2->id()) > 0);
    EXPECT_TRUE(used.count(add->id()) > 0);
}

TEST_F(CodegenOptimizerTest, InferTypes_LiteralTypesAreCorrect) {
    // Create literals of different types
    auto *int_lit = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    int_lit->set_type(NodeBase::PinDataType::kInt);
    int_lit->set_data(42);
    
    auto *float_lit = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    float_lit->set_type(NodeBase::PinDataType::kFloat);
    float_lit->set_data(3.14);
    
    auto *bool_lit = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    bool_lit->set_type(NodeBase::PinDataType::kBool);
    bool_lit->set_data(true);
    
    CodegenOptimizer optimizer;
    std::unordered_map<uint32_t, NodeBase::PinDataType> types;
    optimizer.InferTypes(graph_, types);
    
    EXPECT_EQ(types[int_lit->id()], NodeBase::PinDataType::kInt);
    EXPECT_EQ(types[float_lit->id()], NodeBase::PinDataType::kFloat);
    EXPECT_EQ(types[bool_lit->id()], NodeBase::PinDataType::kBool);
}

TEST_F(CodegenOptimizerTest, InferTypes_ComparisonReturnsBoolean) {
    // Create: lit1 = 5, lit2 = 3, cmp = lit1 == lit2
    auto *lit1 = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    lit1->set_data(5);
    
    auto *lit2 = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    lit2->set_data(3);
    
    auto *cmp = graph_.AddNode<OperatorNode>(NodeBase::NodeKind::kOperator);
    cmp->set_operator_type(OperatorNode::OperatorType::kEqual);
    
    graph_.Link(lit1, 0, cmp, 0);
    graph_.Link(lit2, 0, cmp, 1);
    
    CodegenOptimizer optimizer;
    std::unordered_map<uint32_t, NodeBase::PinDataType> types;
    optimizer.InferTypes(graph_, types);
    
    // Comparison should return bool
    EXPECT_EQ(types[cmp->id()], NodeBase::PinDataType::kBool);
}

TEST_F(CodegenOptimizerTest, InferTypes_TypePromotion_IntTimesInt) {
    // Create: int_lit1 = 5, int_lit2 = 3, mul = int_lit1 * int_lit2
    auto *int_lit1 = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    int_lit1->set_type(NodeBase::PinDataType::kInt);
    int_lit1->set_data(5);
    
    auto *int_lit2 = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    int_lit2->set_type(NodeBase::PinDataType::kInt);
    int_lit2->set_data(3);
    
    auto *mul = graph_.AddNode<OperatorNode>(NodeBase::NodeKind::kOperator);
    mul->set_operator_type(OperatorNode::OperatorType::kMultiplication);
    
    graph_.Link(int_lit1, 0, mul, 0);
    graph_.Link(int_lit2, 0, mul, 1);
    
    CodegenOptimizer optimizer;
    std::unordered_map<uint32_t, NodeBase::PinDataType> types;
    optimizer.InferTypes(graph_, types);
    
    // Result should be int (both inputs are int)
    EXPECT_EQ(types[mul->id()], NodeBase::PinDataType::kInt);
}

TEST_F(CodegenOptimizerTest, AnalyzeGraph_ReturnsCompleteAnalysis) {
    // Create: lit1 = 5, lit2 = 3, add = lit1 + lit2
    auto *lit1 = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    lit1->set_data(5);
    
    auto *lit2 = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    lit2->set_data(3);
    
    auto *add = graph_.AddNode<OperatorNode>(NodeBase::NodeKind::kOperator);
    add->set_operator_type(OperatorNode::OperatorType::kAddition);
    
    graph_.Link(lit1, 0, add, 0);
    graph_.Link(lit2, 0, add, 1);
    
    CodegenOptimizer optimizer;
    auto analysis = optimizer.AnalyzeGraph(graph_);
    
    // All nodes should be used, no dead code
    EXPECT_EQ(analysis.dead_nodes.size(), 0);
    
    // All nodes should have type info
    EXPECT_EQ(analysis.type_info.size(), 3);
    EXPECT_TRUE(analysis.type_info[lit1->id()].is_constant);
    EXPECT_TRUE(analysis.type_info[lit2->id()].is_constant);
    EXPECT_FALSE(analysis.type_info[add->id()].is_constant);
    EXPECT_TRUE(analysis.type_info[add->id()].is_used);
}

TEST_F(CodegenOptimizerTest, LogicalNotUnaryOperator) {
    // Create: bool_lit = true, not_op = !bool_lit
    auto *bool_lit = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    bool_lit->set_type(NodeBase::PinDataType::kBool);
    bool_lit->set_data(true);
    
    auto *not_op = graph_.AddNode<OperatorNode>(NodeBase::NodeKind::kOperator);
    not_op->set_operator_type(OperatorNode::OperatorType::kLogicalNot);
    
    graph_.Link(bool_lit, 0, not_op, 0);
    
    CodegenOptimizer optimizer;
    std::unordered_map<uint32_t, NodeBase::PinDataType> types;
    optimizer.InferTypes(graph_, types);
    
    // Logical not should return bool
    EXPECT_EQ(types[not_op->id()], NodeBase::PinDataType::kBool);
}

TEST_F(CodegenOptimizerTest, BitwiseNotUnaryOperator) {
    // Create: int_lit = 5, bitnot_op = ~int_lit
    auto *int_lit = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    int_lit->set_data(5);
    
    auto *bitnot_op = graph_.AddNode<OperatorNode>(NodeBase::NodeKind::kOperator);
    bitnot_op->set_operator_type(OperatorNode::OperatorType::kBitwiseNot);
    
    graph_.Link(int_lit, 0, bitnot_op, 0);
    
    CodegenOptimizer optimizer;
    std::unordered_map<uint32_t, NodeBase::PinDataType> types;
    optimizer.InferTypes(graph_, types);
    
    // Bitwise not should preserve input type (int)
    EXPECT_EQ(types[bitnot_op->id()], NodeBase::PinDataType::kInt);
}

TEST_F(CodegenOptimizerTest, LogicalAnd_ReturnsBool) {
    // Create: lit1 = true, lit2 = false, and_op = lit1 && lit2
    auto *lit1 = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    lit1->set_type(NodeBase::PinDataType::kBool);
    lit1->set_data(true);
    
    auto *lit2 = graph_.AddNode<LiteralNode>(NodeBase::NodeKind::kLiteral);
    lit2->set_type(NodeBase::PinDataType::kBool);
    lit2->set_data(false);
    
    auto *and_op = graph_.AddNode<OperatorNode>(NodeBase::NodeKind::kOperator);
    and_op->set_operator_type(OperatorNode::OperatorType::kLogicalAnd);
    
    graph_.Link(lit1, 0, and_op, 0);
    graph_.Link(lit2, 0, and_op, 1);
    
    CodegenOptimizer optimizer;
    std::unordered_map<uint32_t, NodeBase::PinDataType> types;
    optimizer.InferTypes(graph_, types);
    
    // Logical AND should return bool
    EXPECT_EQ(types[and_op->id()], NodeBase::PinDataType::kBool);
}
