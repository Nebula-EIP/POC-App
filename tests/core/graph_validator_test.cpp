/**
 * @file graph_validator_test.cpp
 * @brief Unit tests for core::GraphValidator covering cycles, valid loops,
 * required connections, pin types, property values, and unsupported nodes.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 15-11-2026
 */

#include "graph/graph_validator.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>

#include "graph/graph.hpp"
#include "nodes/c_node_list_capability.hpp"

namespace {

constexpr core::NodeType kVarType = 1;
constexpr core::NodeType kLitType = 2;
constexpr core::NodeType kOpType = 3;
constexpr core::NodeType kCondType = 4;
constexpr core::NodeType kLoopType = 5;
constexpr core::NodeType kFuncType = 6;
constexpr core::NodeType kFuncInType = 7;
constexpr core::NodeType kFuncOutType = 8;
constexpr core::NodeType kPrintType = 9;
constexpr core::NodeType kForType = 10;

constexpr core::DataType kVoidData = 1;
constexpr core::DataType kIntData = 2;
constexpr core::DataType kFloatData = 3;
constexpr core::DataType kBoolData = 6;
constexpr core::DataType kStringData = 7;

class GraphValidatorTest : public ::testing::Test {
   protected:
    core::Node &MakeLiteral(core::DataType dtype = kIntData,
                            std::string type_str = "int",
                            std::any val = 42.0) {
        core::Node &n = graph_.CreateNode(kLitType);
        graph_.AddOutputPin(n.Id(), "out", dtype);
        n.SetProperty(c_module::properties::kType,
                      {c_module::properties::kType, std::move(type_str)});
        n.SetProperty(c_module::properties::kValue,
                      {c_module::properties::kValue, std::move(val)});
        return n;
    }

    core::Node &MakeVariable(std::string name = "x",
                             core::DataType dtype = kIntData,
                             std::string type_str = "int") {
        core::Node &n = graph_.CreateNode(kVarType);
        graph_.AddInputPin(n.Id(), "in", dtype);
        graph_.AddOutputPin(n.Id(), "out", dtype);
        n.SetProperty(c_module::properties::kName,
                      {c_module::properties::kName, std::move(name)});
        n.SetProperty(c_module::properties::kType,
                      {c_module::properties::kType, std::move(type_str)});
        return n;
    }

    core::Node &MakeOperator(std::string op = "+",
                             core::DataType in_type = kIntData,
                             core::DataType out_type = kIntData,
                             std::string type_str = "int") {
        core::Node &n = graph_.CreateNode(kOpType);
        graph_.AddInputPin(n.Id(), "left", in_type);
        graph_.AddInputPin(n.Id(), "right", in_type);
        graph_.AddOutputPin(n.Id(), "result", out_type);
        n.SetProperty(c_module::properties::kOperator,
                      {c_module::properties::kOperator, std::move(op)});
        n.SetProperty(c_module::properties::kType,
                      {c_module::properties::kType, std::move(type_str)});
        return n;
    }

    core::Node &MakeFor(std::string name = "i") {
        core::Node &n = graph_.CreateNode(kForType);
        graph_.AddInputPin(n.Id(), "in", kVoidData);
        graph_.AddInputPin(n.Id(), "init", kIntData);
        graph_.AddInputPin(n.Id(), "cond", kBoolData);
        graph_.AddInputPin(n.Id(), "step", kIntData);
        graph_.AddOutputPin(n.Id(), "body", kVoidData);
        graph_.AddOutputPin(n.Id(), "index", kIntData);
        graph_.AddOutputPin(n.Id(), "exit", kVoidData);
        n.SetProperty(c_module::properties::kName,
                      {c_module::properties::kName, std::move(name)});
        return n;
    }

    core::Node &MakeLoop() {
        core::Node &n = graph_.CreateNode(kLoopType);
        graph_.AddInputPin(n.Id(), "in", kVoidData);
        graph_.AddInputPin(n.Id(), "cond", kBoolData);
        graph_.AddOutputPin(n.Id(), "body", kVoidData);
        graph_.AddOutputPin(n.Id(), "exit", kVoidData);
        return n;
    }

    core::Node &MakeCondition() {
        core::Node &n = graph_.CreateNode(kCondType);
        graph_.AddInputPin(n.Id(), "in", kVoidData);
        graph_.AddInputPin(n.Id(), "cond", kBoolData);
        graph_.AddOutputPin(n.Id(), "true", kVoidData);
        graph_.AddOutputPin(n.Id(), "false", kVoidData);
        graph_.AddOutputPin(n.Id(), "out", kVoidData);
        n.SetProperty(c_module::properties::kCondition,
                      {c_module::properties::kCondition, false});
        return n;
    }

    core::Node &MakePrint(core::DataType dtype = kIntData,
                          std::string type_str = "int") {
        core::Node &n = graph_.CreateNode(kPrintType);
        graph_.AddInputPin(n.Id(), "control", kVoidData);
        graph_.AddInputPin(n.Id(), "value", dtype);
        graph_.AddOutputPin(n.Id(), "out", kVoidData);
        n.SetProperty(c_module::properties::kType,
                      {c_module::properties::kType, std::move(type_str)});
        return n;
    }

    core::Graph graph_;
    core::GraphValidator validator_;
};

}  // namespace

// ============================================================================
// Basic Valid Graphs & Determinism
// ============================================================================

TEST_F(GraphValidatorTest, EmptyGraphIsValid) {
    const auto kResult = validator_.Validate(graph_);
    EXPECT_TRUE(kResult.is_valid_);
    EXPECT_EQ(kResult.ErrorCount(), 0U);
}

TEST_F(GraphValidatorTest, SingleNodeNoConnectionsIsValid) {
    MakeLiteral();
    const auto kResult = validator_.Validate(graph_);
    EXPECT_TRUE(kResult.is_valid_);
    EXPECT_EQ(kResult.ErrorCount(), 0U);
}

TEST_F(GraphValidatorTest, SimpleLinearGraphIsValidConsistently) {
    core::Node &lit1 = MakeLiteral(kIntData, "int", 10.0);
    core::Node &lit2 = MakeLiteral(kIntData, "int", 20.0);
    core::Node &op = MakeOperator("+", kIntData, kIntData, "int");
    core::Node &var = MakeVariable("sum", kIntData, "int");

    graph_.Connect(lit1.Id(), 1, op.Id(), 1);
    graph_.Connect(lit2.Id(), 1, op.Id(), 2);
    graph_.Connect(op.Id(), 1, var.Id(), 1);

    for (int run = 0; run < 5; ++run) {
        const auto kResult = validator_.Validate(graph_);
        EXPECT_TRUE(kResult.is_valid_);
        EXPECT_EQ(kResult.ErrorCount(), 0U);
    }
}

// ============================================================================
// Loop Validation (Valid Loops vs Invalid Cycles)
// ============================================================================

TEST_F(GraphValidatorTest, ValidForLoopWithIndexFeedingConditionAndBodyPasses) {
    core::Node &for_node = MakeFor("i");
    core::Node &lit_init = MakeLiteral(kIntData, "int", 0.0);
    core::Node &lit_limit = MakeLiteral(kIntData, "int", 10.0);
    core::Node &lit_step = MakeLiteral(kIntData, "int", 1.0);
    core::Node &cmp_op = MakeOperator("<", kIntData, kBoolData, "int");
    core::Node &print_node = MakePrint(kIntData, "int");

    // init = 0, step = 1
    graph_.Connect(lit_init.Id(), 1, for_node.Id(), 2);
    graph_.Connect(lit_step.Id(), 1, for_node.Id(), 4);

    // For.index (out pin 2) -> cmp_op.left (in pin 1)
    // lit_limit (out pin 1) -> cmp_op.right (in pin 2)
    // cmp_op.result (out pin 1) -> For.cond (in pin 3)
    graph_.Connect(for_node.Id(), 2, cmp_op.Id(), 1);
    graph_.Connect(lit_limit.Id(), 1, cmp_op.Id(), 2);
    graph_.Connect(cmp_op.Id(), 1, for_node.Id(), 3);

    // For.body (out pin 1) -> print_node.control (in pin 1)
    // For.index (out pin 2) -> print_node.value (in pin 2)
    graph_.Connect(for_node.Id(), 1, print_node.Id(), 1);
    graph_.Connect(for_node.Id(), 2, print_node.Id(), 2);

    const auto kResult = validator_.Validate(graph_);
    EXPECT_TRUE(kResult.is_valid_);
    EXPECT_FALSE(kResult.HasCycleError());
    EXPECT_EQ(kResult.ErrorCount(), 0U);
}

TEST_F(GraphValidatorTest, ValidWhileLoopWithBodyFeedingConditionPasses) {
    core::Node &loop_node = MakeLoop();
    core::Node &lit_limit = MakeLiteral(kIntData, "int", 5.0);
    core::Node &var_node = MakeVariable("counter", kIntData, "int");
    core::Node &cmp_op = MakeOperator("<", kIntData, kBoolData, "int");
    core::Node &print_node = MakePrint(kIntData, "int");

    // var_node.out -> cmp_op.left, lit_limit.out -> cmp_op.right
    // cmp_op.result -> loop_node.cond (in pin 2)
    graph_.Connect(var_node.Id(), 1, cmp_op.Id(), 1);
    graph_.Connect(lit_limit.Id(), 1, cmp_op.Id(), 2);
    graph_.Connect(cmp_op.Id(), 1, loop_node.Id(), 2);

    // loop_node.body -> print_node.control, var_node.out -> print_node.value
    graph_.Connect(loop_node.Id(), 1, print_node.Id(), 1);
    graph_.Connect(var_node.Id(), 1, print_node.Id(), 2);

    const auto kResult = validator_.Validate(graph_);
    EXPECT_TRUE(kResult.is_valid_);
    EXPECT_FALSE(kResult.HasCycleError());
}

TEST_F(GraphValidatorTest, InvalidCycleFromForIndexToForInitIsDetected) {
    core::Node &for_node = MakeFor("i");
    core::Node &lit_one = MakeLiteral(kIntData, "int", 1.0);
    core::Node &lit_true = MakeLiteral(kBoolData, "bool", true);
    core::Node &add_op = MakeOperator("+", kIntData, kIntData, "int");

    // For.index (out 2) -> add_op.left (in 1), lit_one -> add_op.right (in 2)
    // add_op.result -> For.init (in 2) -- INVALID! init cannot depend on index!
    graph_.Connect(for_node.Id(), 2, add_op.Id(), 1);
    graph_.Connect(lit_one.Id(), 1, add_op.Id(), 2);
    graph_.Connect(add_op.Id(), 1, for_node.Id(), 2);

    graph_.Connect(lit_true.Id(), 1, for_node.Id(), 3);
    graph_.Connect(lit_one.Id(), 1, for_node.Id(), 4);

    const auto kResult = validator_.Validate(graph_);
    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasCycleError());
}

TEST_F(GraphValidatorTest, InvalidCycleFromForExitToForInIsDetected) {
    core::Node &for_node = MakeFor("i");
    core::Node &lit_zero = MakeLiteral(kIntData, "int", 0.0);
    core::Node &lit_true = MakeLiteral(kBoolData, "bool", true);
    core::Node &lit_one = MakeLiteral(kIntData, "int", 1.0);
    core::Node &print_node = MakePrint(kIntData, "int");

    graph_.Connect(lit_zero.Id(), 1, for_node.Id(), 2);
    graph_.Connect(lit_true.Id(), 1, for_node.Id(), 3);
    graph_.Connect(lit_one.Id(), 1, for_node.Id(), 4);
    graph_.Connect(lit_zero.Id(), 1, print_node.Id(), 2);

    // For.exit (out 3) -> print_node.control (in 1)
    // print_node.out (out 1) -> For.in (in 1) -- INVALID cycle!
    graph_.Connect(for_node.Id(), 3, print_node.Id(), 1);
    graph_.Connect(print_node.Id(), 1, for_node.Id(), 1);

    const auto kResult = validator_.Validate(graph_);
    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasCycleError());
}

TEST_F(GraphValidatorTest, DirectSelfLoopDetected) {
    core::Node &var_a = MakeVariable("a");
    graph_.Connect(var_a.Id(), 1, var_a.Id(), 1);

    const auto kResult = validator_.Validate(graph_);
    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasCycleError());
    ASSERT_FALSE(kResult.errors_.empty());
    EXPECT_NE(std::find(kResult.errors_[0].involved_node_ids_.begin(),
                        kResult.errors_[0].involved_node_ids_.end(),
                        var_a.Id()),
              kResult.errors_[0].involved_node_ids_.end());
}

TEST_F(GraphValidatorTest, TwoNodeCycleDetected) {
    core::Node &var_a = MakeVariable("a");
    core::Node &var_b = MakeVariable("b");

    graph_.Connect(var_a.Id(), 1, var_b.Id(), 1);
    graph_.Connect(var_b.Id(), 1, var_a.Id(), 1);

    const auto kResult = validator_.Validate(graph_);
    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasCycleError());
}

TEST_F(GraphValidatorTest, MultiNodeCycleDetected) {
    core::Node &var_a = MakeVariable("a");
    core::Node &var_b = MakeVariable("b");
    core::Node &var_c = MakeVariable("c");

    graph_.Connect(var_a.Id(), 1, var_b.Id(), 1);
    graph_.Connect(var_b.Id(), 1, var_c.Id(), 1);
    graph_.Connect(var_c.Id(), 1, var_a.Id(), 1);

    const auto kResult = validator_.Validate(graph_);
    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasCycleError());
}

// ============================================================================
// Missing Required Connections
// ============================================================================

TEST_F(GraphValidatorTest, MissingOperatorInputsDetected) {
    core::Node &lit = MakeLiteral();
    core::Node &op = MakeOperator();

    // Connect only left (in 1), leave right (in 2) disconnected
    graph_.Connect(lit.Id(), 1, op.Id(), 1);

    const auto kResult = validator_.Validate(graph_);
    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasPinConnectivityError());
    ASSERT_EQ(kResult.ErrorCount(), 1U);
    EXPECT_EQ(kResult.errors_[0].involved_node_ids_.front(), op.Id());
    ASSERT_EQ(kResult.errors_[0].involved_pins_.size(), 1U);
    EXPECT_EQ(kResult.errors_[0].involved_pins_[0].pin_id_, 2U);
}

TEST_F(GraphValidatorTest, MissingConditionAndLoopAndPrintInputsDetected) {
    core::Node &cond = MakeCondition();
    core::Node &loop = MakeLoop();
    core::Node &print = MakePrint();
    (void)cond;
    (void)loop;
    (void)print;

    const auto kResult = validator_.Validate(graph_);
    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasPinConnectivityError());
    EXPECT_EQ(kResult.ErrorCount(), 3U);
}

// ============================================================================
// Incompatible Types, Incompatible Values & Unsupported Nodes
// ============================================================================

TEST_F(GraphValidatorTest, UnsupportedNodeTypeReported) {
    core::Node &unsupported = graph_.CreateNode(999);
    const auto kResult = validator_.Validate(graph_);

    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasUnsupportedNodeError());
    ASSERT_EQ(kResult.ErrorCount(), 1U);
    EXPECT_EQ(kResult.errors_[0].involved_node_ids_.front(), unsupported.Id());
}

TEST_F(GraphValidatorTest, IncompatiblePinTypesWhenPropertyChangedReported) {
    core::Node &lit = MakeLiteral(kIntData, "int", 10.0);
    core::Node &var = MakeVariable("x", kIntData, "int");
    graph_.Connect(lit.Id(), 1, var.Id(), 1);

    // Change literal property to string while pin connection remains int
    lit.SetProperty(c_module::properties::kType,
                    {c_module::properties::kType, std::string("string")});
    lit.SetProperty(c_module::properties::kValue,
                    {c_module::properties::kValue, std::string("hello")});

    const auto kResult = validator_.Validate(graph_);
    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasTypeError());
}

TEST_F(GraphValidatorTest, IncompatibleValuesInPropertiesReported) {
    // 1. Variable with reserved keyword name
    MakeVariable("while", kIntData, "int");
    // 2. Operator with string type and '-' operator (invalid in C++)
    core::Node &str_a = MakeLiteral(kStringData, "string", std::string("a"));
    core::Node &str_b = MakeLiteral(kStringData, "string", std::string("b"));
    core::Node &bad_op = MakeOperator("-", kStringData, kStringData, "string");
    graph_.Connect(str_a.Id(), 1, bad_op.Id(), 1);
    graph_.Connect(str_b.Id(), 1, bad_op.Id(), 2);

    const auto kResult = validator_.Validate(graph_);
    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasValueError());
    EXPECT_GE(kResult.ErrorCount(), 2U);
}

TEST_F(GraphValidatorTest, DivisionByZeroLiteralReported) {
    core::Node &lhs = MakeLiteral(kIntData, "int", 10.0);
    core::Node &zero = MakeLiteral(kIntData, "int", 0.0);
    core::Node &div_op = MakeOperator("/", kIntData, kIntData, "int");

    graph_.Connect(lhs.Id(), 1, div_op.Id(), 1);
    graph_.Connect(zero.Id(), 1, div_op.Id(), 2);

    const auto kResult = validator_.Validate(graph_);
    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasValueError());
}
