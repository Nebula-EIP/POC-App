#include <gtest/gtest.h>

#include "core/graph.hpp"
#include "core/graph_validator.hpp"
#include "core/nodes/literal_node.hpp"
#include "core/nodes/operator_node.hpp"
#include "core/nodes/variable_node.hpp"

class GraphValidatorTest : public ::testing::Test {
   protected:
    core::Graph graph_;
    core::GraphValidator validator_;
};

// ============================================================================
// Cycle Detection Tests
// ============================================================================

TEST_F(GraphValidatorTest, EmptyGraphIsValid) {
    auto result = validator_.Validate(graph_);
    EXPECT_TRUE(result.is_valid);
    EXPECT_EQ(result.ErrorCount(), 0);
}

TEST_F(GraphValidatorTest, SimpleLinearGraphIsValid) {
    // Create: Lit1 → Op → Lit2
    auto *lit1 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *lit2 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *op = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});

    ASSERT_NE(lit1, nullptr);
    ASSERT_NE(lit2, nullptr);
    ASSERT_NE(op, nullptr);

    lit1->SetType(core::NodeBase::PinDataType::kInt);
    lit2->SetType(core::NodeBase::PinDataType::kInt);
    op->SetOperatorType(core::OperatorNode::OperatorType::kAddition);

    graph_.Link(lit1, 0, op, 0);
    graph_.Link(lit2, 0, op, 1);

    auto result = validator_.Validate(graph_);
    EXPECT_TRUE(result.is_valid);
    EXPECT_EQ(result.ErrorCount(), 0);
}

TEST_F(GraphValidatorTest, SimpleDirectCycleDetected) {
    // Create a simple cycle: A → A (self-loop)
    auto *node_a = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable, {0, 0});
    ASSERT_NE(node_a, nullptr);

    // Try to link to itself
    EXPECT_THROW(graph_.Link(node_a, 0, node_a, 0), core::CircularDependencyException);
}

TEST_F(GraphValidatorTest, TwoNodeCycleDetected) {
    // Create a cycle: A → B → A
    auto *node_a = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *node_b = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable, {0, 0});

    ASSERT_NE(node_a, nullptr);
    ASSERT_NE(node_b, nullptr);

    graph_.Link(node_a, 0, node_b, 0);

    // Now link B back to A to create cycle
    // The graph's Link method should prevent this
    EXPECT_THROW(graph_.Link(node_b, 0, node_a, 0), core::CircularDependencyException);
}

TEST_F(GraphValidatorTest, ComplexCycleDetected) {
    // Create: A → B → C → D → B (cycle in B, C, D)
    auto *node_a = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *node_b = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *node_c = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable, {0, 0});
    auto *node_d = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable, {0, 0});

    ASSERT_NE(node_a, nullptr);
    ASSERT_NE(node_b, nullptr);
    ASSERT_NE(node_c, nullptr);
    ASSERT_NE(node_d, nullptr);

    graph_.Link(node_a, 0, node_b, 0);
    graph_.Link(node_b, 0, node_c, 0);
    graph_.Link(node_c, 0, node_d, 0);

    // Try to create cycle: D → B
    EXPECT_THROW(graph_.Link(node_d, 0, node_b, 0), core::CircularDependencyException);
}

TEST_F(GraphValidatorTest, MultipleIndependentPathsValid) {
    // Create: Lit1 → Op1, Lit2 → Op2, both independent
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

    graph_.Link(lit1, 0, op1, 0);
    graph_.Link(lit1, 0, op1, 1);
    graph_.Link(lit2, 0, op2, 0);
    graph_.Link(lit2, 0, op2, 1);

    auto result = validator_.Validate(graph_);
    EXPECT_TRUE(result.is_valid);
    EXPECT_EQ(result.ErrorCount(), 0);
}

// ============================================================================
// Pin Type Compatibility Tests
// ============================================================================

TEST_F(GraphValidatorTest, CompatiblePinTypesValid) {
    // Int → Int should be valid
    auto *lit = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *var = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable, {0, 0});

    ASSERT_NE(lit, nullptr);
    ASSERT_NE(var, nullptr);

    lit->SetType(core::NodeBase::PinDataType::kInt);
    var->SetType(core::NodeBase::PinDataType::kInt);

    graph_.Link(lit, 0, var, 0);

    auto result = validator_.Validate(graph_);
    EXPECT_TRUE(result.is_valid);
    EXPECT_EQ(result.ErrorCount(), 0);
}

TEST_F(GraphValidatorTest, ImplicitIntToFloatConversionValid) {
    // Int → Float should be implicitly convertible
    auto *lit = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *var = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable, {0, 0});

    ASSERT_NE(lit, nullptr);
    ASSERT_NE(var, nullptr);

    lit->SetType(core::NodeBase::PinDataType::kInt);
    var->SetType(core::NodeBase::PinDataType::kFloat);

    graph_.Link(lit, 0, var, 0);

    auto result = validator_.Validate(graph_);
    EXPECT_TRUE(result.is_valid);
    EXPECT_EQ(result.ErrorCount(), 0);
}

TEST_F(GraphValidatorTest, IncompatibleBoolToIntDetected) {
    // Bool → Int should not be compatible
    auto *lit = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *var = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable, {0, 0});

    ASSERT_NE(lit, nullptr);
    ASSERT_NE(var, nullptr);

    lit->SetType(core::NodeBase::PinDataType::kBool);
    var->SetType(core::NodeBase::PinDataType::kInt);

    EXPECT_THROW(graph_.Link(lit, 0, var, 0), core::IncompatiblePinTypesException);
}

TEST_F(GraphValidatorTest, IncompatibleStringToIntDetected) {
    // String → Int should not be compatible
    auto *lit = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *var = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable, {0, 0});

    ASSERT_NE(lit, nullptr);
    ASSERT_NE(var, nullptr);

    lit->SetType(core::NodeBase::PinDataType::kString);
    var->SetType(core::NodeBase::PinDataType::kInt);

    EXPECT_THROW(graph_.Link(lit, 0, var, 0), core::IncompatiblePinTypesException);
}

TEST_F(GraphValidatorTest, MultipleConnectionsTypeCompatibility) {
    // Create: Lit1 → Op, Lit2 → Op (both int)
    auto *lit1 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *lit2 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *op = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});

    ASSERT_NE(lit1, nullptr);
    ASSERT_NE(lit2, nullptr);
    ASSERT_NE(op, nullptr);

    lit1->SetType(core::NodeBase::PinDataType::kInt);
    lit2->SetType(core::NodeBase::PinDataType::kFloat);
    op->SetOperatorType(core::OperatorNode::OperatorType::kAddition);

    graph_.Link(lit1, 0, op, 0);
    graph_.Link(lit2, 0, op, 1);

    auto result = validator_.Validate(graph_);
    EXPECT_TRUE(result.is_valid);
    EXPECT_EQ(result.ErrorCount(), 0);
}

// ============================================================================
// Pin Connectivity Tests
// ============================================================================

TEST_F(GraphValidatorTest, UnconnectedInputPinDetected) {
    // Create an operator with only one input connected
    auto *lit = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *op = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});

    ASSERT_NE(lit, nullptr);
    ASSERT_NE(op, nullptr);

    lit->SetType(core::NodeBase::PinDataType::kInt);
    op->SetOperatorType(core::OperatorNode::OperatorType::kAddition);

    // Only connect to pin 0, not pin 1
    graph_.Link(lit, 0, op, 0);

    auto result = validator_.Validate(graph_);
    EXPECT_FALSE(result.is_valid);
    EXPECT_TRUE(result.HasPinConnectivityError());
    EXPECT_GT(result.ErrorCount(), 0);
}

TEST_F(GraphValidatorTest, AllInputsConnectedValid) {
    // Create: Lit1 → Op.pin0, Lit2 → Op.pin1
    auto *lit1 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *lit2 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *op = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});

    ASSERT_NE(lit1, nullptr);
    ASSERT_NE(lit2, nullptr);
    ASSERT_NE(op, nullptr);

    lit1->SetType(core::NodeBase::PinDataType::kInt);
    lit2->SetType(core::NodeBase::PinDataType::kInt);
    op->SetOperatorType(core::OperatorNode::OperatorType::kAddition);

    graph_.Link(lit1, 0, op, 0);
    graph_.Link(lit2, 0, op, 1);

    auto result = validator_.Validate(graph_);
    EXPECT_TRUE(result.is_valid);
    EXPECT_EQ(result.ErrorCount(), 0);
}

// ============================================================================
// Complex Validation Scenarios
// ============================================================================

TEST_F(GraphValidatorTest, MultipleErrorsReported) {
    // Create a scenario with multiple types of errors
    auto *lit1 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *lit2 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *op = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});

    ASSERT_NE(lit1, nullptr);
    ASSERT_NE(lit2, nullptr);
    ASSERT_NE(op, nullptr);

    lit1->SetType(core::NodeBase::PinDataType::kBool);
    lit2->SetType(core::NodeBase::PinDataType::kInt);
    op->SetOperatorType(core::OperatorNode::OperatorType::kAddition);

    // Try incompatible connection
    EXPECT_THROW(graph_.Link(lit1, 0, op, 0), core::IncompatiblePinTypesException);

    // Connection failed, so only lit2 is connected
    graph_.Link(lit2, 0, op, 1);

    auto result = validator_.Validate(graph_);
    EXPECT_FALSE(result.is_valid);
    EXPECT_TRUE(result.HasPinConnectivityError());
}

TEST_F(GraphValidatorTest, LargeGraphValidation) {
    // Create a larger graph to test DFS performance
    std::vector<core::LiteralNode *> literals;
    std::vector<core::OperatorNode *> operators;

    // Create 10 literals
    for (int i = 0; i < 10; ++i) {
        auto *lit = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
        ASSERT_NE(lit, nullptr);
        lit->SetType(core::NodeBase::PinDataType::kInt);
        literals.push_back(lit);
    }

    // Create 5 operators in a chain
    for (int i = 0; i < 5; ++i) {
        auto *op = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
        ASSERT_NE(op, nullptr);
        op->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
        operators.push_back(op);
    }

    // Create connections: connect literals to first operators, then chain operators
    for (int i = 0; i < 2; ++i) {
        graph_.Link(literals[i], 0, operators[0], i);
    }
    for (int i = 1; i < 5; ++i) {
        graph_.Link(operators[i - 1], 0, operators[i], 0);
        graph_.Link(literals[i + 1], 0, operators[i], 1);
    }

    auto result = validator_.Validate(graph_);
    EXPECT_TRUE(result.is_valid);
    EXPECT_EQ(result.ErrorCount(), 0);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(GraphValidatorTest, SingleNodeNoConnectionsValid) {
    auto *lit = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    ASSERT_NE(lit, nullptr);

    auto result = validator_.Validate(graph_);
    EXPECT_TRUE(result.is_valid);
}

TEST_F(GraphValidatorTest, ValidateReportsCorrectNodeIds) {
    auto *lit1 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *lit2 = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *op = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});

    ASSERT_NE(lit1, nullptr);
    ASSERT_NE(lit2, nullptr);
    ASSERT_NE(op, nullptr);

    lit1->SetType(core::NodeBase::PinDataType::kInt);
    lit2->SetType(core::NodeBase::PinDataType::kInt);
    op->SetOperatorType(core::OperatorNode::OperatorType::kAddition);

    graph_.Link(lit1, 0, op, 0);
    // Don't connect op pin 1

    auto result = validator_.Validate(graph_);
    EXPECT_FALSE(result.is_valid);

    // Check that error contains correct node IDs
    ASSERT_GT(result.errors.size(), 0);
    bool found_op_error = false;
    for (const auto &error : result.errors) {
        if (std::find(error.involved_node_ids.begin(), error.involved_node_ids.end(),
                      op->id()) != error.involved_node_ids.end()) {
            found_op_error = true;
            break;
        }
    }
    EXPECT_TRUE(found_op_error);
}
