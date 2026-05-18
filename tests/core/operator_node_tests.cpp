#include <gtest/gtest.h>

#include "graph.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/operator_node.hpp"

class OperatorNodeTest : public testing::Test {
   protected:
    core::Graph graph_;
};

TEST_F(OperatorNodeTest, CreateOperatorNode_ValidKind_Success) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kOperator);
}

TEST_F(OperatorNodeTest, DefaultOperatorType_IsAddition) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->operator_type(), core::OperatorNode::OperatorType::kAddition);
}

TEST_F(OperatorNodeTest, SetOperatorType_Changes) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});

    node->SetOperatorType(core::OperatorNode::OperatorType::kMultiplication);
    EXPECT_EQ(node->operator_type(), core::OperatorNode::OperatorType::kMultiplication);

    node->SetOperatorType(core::OperatorNode::OperatorType::kLogicalAnd);
    EXPECT_EQ(node->operator_type(), core::OperatorNode::OperatorType::kLogicalAnd);
}

TEST_F(OperatorNodeTest, SetName_ChangesName) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    node->SetName("CustomAdd");
    EXPECT_EQ(node->Name(), "CustomAdd");
    EXPECT_EQ(node->GetDisplayName(), "CustomAdd");
}

TEST_F(OperatorNodeTest, BinaryOperator_HasTwoInputs) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    node->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    EXPECT_EQ(node->GetInputPinCount(), 2);

    node->SetOperatorType(core::OperatorNode::OperatorType::kMultiplication);
    EXPECT_EQ(node->GetInputPinCount(), 2);

    node->SetOperatorType(core::OperatorNode::OperatorType::kLogicalOr);
    EXPECT_EQ(node->GetInputPinCount(), 2);
}

TEST_F(OperatorNodeTest, UnaryOperator_HasOneInput) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    node->SetOperatorType(core::OperatorNode::OperatorType::kBitwiseNot);
    EXPECT_EQ(node->GetInputPinCount(), 1);

    node->SetOperatorType(core::OperatorNode::OperatorType::kLogicalNot);
    EXPECT_EQ(node->GetInputPinCount(), 1);
}

TEST_F(OperatorNodeTest, AllOperators_HaveOneOutput) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    node->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    EXPECT_EQ(node->GetOutputPinCount(), 1);

    node->SetOperatorType(core::OperatorNode::OperatorType::kLogicalNot);
    EXPECT_EQ(node->GetOutputPinCount(), 1);

    node->SetOperatorType(core::OperatorNode::OperatorType::kEqual);
    EXPECT_EQ(node->GetOutputPinCount(), 1);
}

TEST_F(OperatorNodeTest, ArithmeticOperators_IntInputIntOutput) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    std::vector<core::OperatorNode::OperatorType> arithmetic_ops = {
        core::OperatorNode::OperatorType::kAddition,
        core::OperatorNode::OperatorType::kSubtraction,
        core::OperatorNode::OperatorType::kMultiplication,
        core::OperatorNode::OperatorType::kDivision,
        core::OperatorNode::OperatorType::kModulo
    };

    for (auto op : arithmetic_ops) {
        node->SetOperatorType(op);
        EXPECT_EQ(node->GetInputPinType(0), core::NodeBase::PinDataType::kInt);
        EXPECT_EQ(node->GetInputPinType(1), core::NodeBase::PinDataType::kInt);
        EXPECT_EQ(node->GetOutputPinType(0), core::NodeBase::PinDataType::kInt);
    }
}

TEST_F(OperatorNodeTest, BitwiseOperators_IntInputIntOutput) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    std::vector<core::OperatorNode::OperatorType> bitwise_ops = {
        core::OperatorNode::OperatorType::kBitwiseAnd,
        core::OperatorNode::OperatorType::kBitwiseOr,
        core::OperatorNode::OperatorType::kBitwiseXor,
        core::OperatorNode::OperatorType::kLeftShift,
        core::OperatorNode::OperatorType::kRightShift
    };

    for (auto op : bitwise_ops) {
        node->SetOperatorType(op);
        EXPECT_EQ(node->GetInputPinType(0), core::NodeBase::PinDataType::kInt);
        EXPECT_EQ(node->GetInputPinType(1), core::NodeBase::PinDataType::kInt);
        EXPECT_EQ(node->GetOutputPinType(0), core::NodeBase::PinDataType::kInt);
    }
}

TEST_F(OperatorNodeTest, BitwiseNot_UnaryIntToInt) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    node->SetOperatorType(core::OperatorNode::OperatorType::kBitwiseNot);
    EXPECT_EQ(node->GetInputPinCount(), 1);
    EXPECT_EQ(node->GetInputPinType(0), core::NodeBase::PinDataType::kInt);
    EXPECT_EQ(node->GetOutputPinType(0), core::NodeBase::PinDataType::kInt);
}

TEST_F(OperatorNodeTest, ComparisonOperators_IntInputBoolOutput) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    std::vector<core::OperatorNode::OperatorType> comparison_ops = {
        core::OperatorNode::OperatorType::kEqual,
        core::OperatorNode::OperatorType::kNotEqual,
        core::OperatorNode::OperatorType::kLessThan,
        core::OperatorNode::OperatorType::kGreaterThan,
        core::OperatorNode::OperatorType::kLessOrEqual,
        core::OperatorNode::OperatorType::kGreaterOrEqual
    };

    for (auto op : comparison_ops) {
        node->SetOperatorType(op);
        EXPECT_EQ(node->GetInputPinType(0), core::NodeBase::PinDataType::kInt);
        EXPECT_EQ(node->GetInputPinType(1), core::NodeBase::PinDataType::kInt);
        EXPECT_EQ(node->GetOutputPinType(0), core::NodeBase::PinDataType::kBool);
    }
}

TEST_F(OperatorNodeTest, LogicalBinaryOperators_BoolInputBoolOutput) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    std::vector<core::OperatorNode::OperatorType> logical_ops = {
        core::OperatorNode::OperatorType::kLogicalAnd,
        core::OperatorNode::OperatorType::kLogicalOr
    };

    for (auto op : logical_ops) {
        node->SetOperatorType(op);
        EXPECT_EQ(node->GetInputPinType(0), core::NodeBase::PinDataType::kBool);
        EXPECT_EQ(node->GetInputPinType(1), core::NodeBase::PinDataType::kBool);
        EXPECT_EQ(node->GetOutputPinType(0), core::NodeBase::PinDataType::kBool);
    }
}

TEST_F(OperatorNodeTest, LogicalNot_UnaryBoolToBool) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    node->SetOperatorType(core::OperatorNode::OperatorType::kLogicalNot);
    EXPECT_EQ(node->GetInputPinCount(), 1);
    EXPECT_EQ(node->GetInputPinType(0), core::NodeBase::PinDataType::kBool);
    EXPECT_EQ(node->GetOutputPinType(0), core::NodeBase::PinDataType::kBool);
}

TEST_F(OperatorNodeTest, BinaryOperator_InputPinNames) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    node->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    EXPECT_EQ(node->GetInputPinName(0), "A");
    EXPECT_EQ(node->GetInputPinName(1), "B");
    EXPECT_EQ(node->GetInputPinName(2), "");
}

TEST_F(OperatorNodeTest, UnaryOperator_InputPinName) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    node->SetOperatorType(core::OperatorNode::OperatorType::kLogicalNot);
    EXPECT_EQ(node->GetInputPinName(0), "Input");
    EXPECT_EQ(node->GetInputPinName(1), "");
}

TEST_F(OperatorNodeTest, OutputPinName_IsResult) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    EXPECT_EQ(node->GetOutputPinName(0), "Result");
    EXPECT_EQ(node->GetOutputPinName(1), "");
}

TEST_F(OperatorNodeTest, ArithmeticOperators_ArithmeticCategory) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    node->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    EXPECT_EQ(node->GetCategory(), "Arithmetic");

    node->SetOperatorType(core::OperatorNode::OperatorType::kDivision);
    EXPECT_EQ(node->GetCategory(), "Arithmetic");
}

TEST_F(OperatorNodeTest, BitwiseOperators_BitwiseCategory) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    node->SetOperatorType(core::OperatorNode::OperatorType::kBitwiseAnd);
    EXPECT_EQ(node->GetCategory(), "Bitwise");

    node->SetOperatorType(core::OperatorNode::OperatorType::kBitwiseNot);
    EXPECT_EQ(node->GetCategory(), "Bitwise");
}

TEST_F(OperatorNodeTest, ComparisonOperators_ComparisonCategory) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    node->SetOperatorType(core::OperatorNode::OperatorType::kEqual);
    EXPECT_EQ(node->GetCategory(), "Comparison");

    node->SetOperatorType(core::OperatorNode::OperatorType::kLessThan);
    EXPECT_EQ(node->GetCategory(), "Comparison");
}

TEST_F(OperatorNodeTest, LogicalOperators_LogicalCategory) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    node->SetOperatorType(core::OperatorNode::OperatorType::kLogicalAnd);
    EXPECT_EQ(node->GetCategory(), "Logical");

    node->SetOperatorType(core::OperatorNode::OperatorType::kLogicalNot);
    EXPECT_EQ(node->GetCategory(), "Logical");
}

TEST_F(OperatorNodeTest, ConnectLiteralsToOperator_ValidTypes_Success) {
    auto* literal_a = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto* literal_b = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto* op_node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    literal_a->SetType(core::NodeBase::PinDataType::kInt);
    literal_b->SetType(core::NodeBase::PinDataType::kInt);
    op_node->SetOperatorType(core::OperatorNode::OperatorType::kAddition);

    EXPECT_NO_THROW(graph_.Link(literal_a, 0, op_node, 0));
    EXPECT_NO_THROW(graph_.Link(literal_b, 0, op_node, 1));
}

TEST_F(OperatorNodeTest, ConnectWrongType_Fails) {
    auto* literal = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto* op_node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    literal->SetType(core::NodeBase::PinDataType::kBool);
    op_node->SetOperatorType(core::OperatorNode::OperatorType::kAddition);

    EXPECT_THROW(graph_.Link(literal, 0, op_node, 0),
                 core::IncompatiblePinTypesException);
}

TEST_F(OperatorNodeTest, ChainOperators_Success) {
    auto* literal_a = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto* literal_b = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto* literal_c = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto* add_node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});
    auto* mul_node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    literal_a->SetType(core::NodeBase::PinDataType::kInt);
    literal_b->SetType(core::NodeBase::PinDataType::kInt);
    literal_c->SetType(core::NodeBase::PinDataType::kInt);
    add_node->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    mul_node->SetOperatorType(core::OperatorNode::OperatorType::kMultiplication);

    EXPECT_NO_THROW(graph_.Link(literal_a, 0, add_node, 0));
    EXPECT_NO_THROW(graph_.Link(literal_b, 0, add_node, 1));
    EXPECT_NO_THROW(graph_.Link(add_node, 0, mul_node, 0));
    EXPECT_NO_THROW(graph_.Link(literal_c, 0, mul_node, 1));
}

TEST_F(OperatorNodeTest, ComparisonChain_IntToBool_Success) {
    auto* literal_a = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto* literal_b = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto* cmp_node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});
    auto* not_node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    literal_a->SetType(core::NodeBase::PinDataType::kInt);
    literal_b->SetType(core::NodeBase::PinDataType::kInt);
    cmp_node->SetOperatorType(core::OperatorNode::OperatorType::kEqual);
    not_node->SetOperatorType(core::OperatorNode::OperatorType::kLogicalNot);

    EXPECT_NO_THROW(graph_.Link(literal_a, 0, cmp_node, 0));
    EXPECT_NO_THROW(graph_.Link(literal_b, 0, cmp_node, 1));
    EXPECT_NO_THROW(graph_.Link(cmp_node, 0, not_node, 0));
}

TEST_F(OperatorNodeTest, Serialize_ContainsRequiredFields) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});
    node->SetOperatorType(core::OperatorNode::OperatorType::kMultiplication);
    node->SetName("Multiply");

    nlohmann::json json = node->Serialize();

    EXPECT_TRUE(json.contains("id"));
    EXPECT_TRUE(json.contains("kind"));
    EXPECT_TRUE(json.contains("operator_type"));
    EXPECT_TRUE(json.contains("name"));

    EXPECT_EQ(json["id"], node->id());
    EXPECT_EQ(json["kind"], "Operator");
    EXPECT_EQ(json["operator_type"], "Multiplication");
    EXPECT_EQ(json["name"], "Multiply");
}

TEST_F(OperatorNodeTest, SerializeDeserialize_PreservesData) {
    auto* original = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});
    original->SetOperatorType(core::OperatorNode::OperatorType::kLogicalAnd);
    original->SetName("MyAndOperator");

    nlohmann::json json = original->Serialize();

    core::Graph new_graph;
    auto result = core::NodeBase::DeserializeFactory(json, &new_graph);

    ASSERT_TRUE(result.has_value());
    auto* deserialized = dynamic_cast<core::OperatorNode*>(result.value().get());
    ASSERT_NE(deserialized, nullptr);

    EXPECT_EQ(deserialized->operator_type(), core::OperatorNode::OperatorType::kLogicalAnd);
    EXPECT_EQ(deserialized->Name(), "MyAndOperator");
}

TEST_F(OperatorNodeTest, Deserialize_MissingFields_Fails) {
    nlohmann::json json;
    json["id"] = 1;
    json["kind"] = "Operator";

    core::Graph new_graph;
    auto* node = new_graph.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});
    auto result = node->Deserialize(json);

    EXPECT_FALSE(result.has_value());
}

TEST_F(OperatorNodeTest, IsUnaryOperator_CorrectForAllTypes) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    node->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    EXPECT_FALSE(node->IsUnaryOperator());

    node->SetOperatorType(core::OperatorNode::OperatorType::kLogicalAnd);
    EXPECT_FALSE(node->IsUnaryOperator());

    node->SetOperatorType(core::OperatorNode::OperatorType::kBitwiseNot);
    EXPECT_TRUE(node->IsUnaryOperator());

    node->SetOperatorType(core::OperatorNode::OperatorType::kLogicalNot);
    EXPECT_TRUE(node->IsUnaryOperator());
}

TEST_F(OperatorNodeTest, OperatorTypeToString_AllTypes) {
    EXPECT_EQ(core::OperatorTypeToString(core::OperatorNode::OperatorType::kAddition), "Addition");
    EXPECT_EQ(core::OperatorTypeToString(core::OperatorNode::OperatorType::kSubtraction), "Subtraction");
    EXPECT_EQ(core::OperatorTypeToString(core::OperatorNode::OperatorType::kBitwiseAnd), "BitwiseAnd");
    EXPECT_EQ(core::OperatorTypeToString(core::OperatorNode::OperatorType::kEqual), "Equal");
    EXPECT_EQ(core::OperatorTypeToString(core::OperatorNode::OperatorType::kLogicalNot), "LogicalNot");
}

TEST_F(OperatorNodeTest, StringToOperatorType_AllTypes) {
    EXPECT_EQ(core::StringToOperatorType("Addition"), core::OperatorNode::OperatorType::kAddition);
    EXPECT_EQ(core::StringToOperatorType("Subtraction"), core::OperatorNode::OperatorType::kSubtraction);
    EXPECT_EQ(core::StringToOperatorType("BitwiseAnd"), core::OperatorNode::OperatorType::kBitwiseAnd);
    EXPECT_EQ(core::StringToOperatorType("Equal"), core::OperatorNode::OperatorType::kEqual);
    EXPECT_EQ(core::StringToOperatorType("LogicalNot"), core::OperatorNode::OperatorType::kLogicalNot);
}

TEST_F(OperatorNodeTest, StringToOperatorType_InvalidString_Throws) {
    EXPECT_THROW(core::StringToOperatorType("InvalidOperator"), std::invalid_argument);
}

TEST_F(OperatorNodeTest, GetInputPinType_InvalidPin_ReturnsUndefined) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});
    node->SetOperatorType(core::OperatorNode::OperatorType::kAddition);

    EXPECT_EQ(node->GetInputPinType(10), core::NodeBase::PinDataType::kUndefined);
}

TEST_F(OperatorNodeTest, GetOutputPinType_InvalidPin_ReturnsUndefined) {
    auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    EXPECT_EQ(node->GetOutputPinType(1), core::NodeBase::PinDataType::kUndefined);
    EXPECT_EQ(node->GetOutputPinType(10), core::NodeBase::PinDataType::kUndefined);
}

TEST_F(OperatorNodeTest, CanConnectTo_InvalidOutputPin_Fails) {
    auto* op_node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});
    auto* target = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    auto result = op_node->CanConnectTo(1, target, 0);

    EXPECT_FALSE(result.has_value());
}

TEST_F(OperatorNodeTest, CanConnectTo_InvalidTargetPin_Fails) {
    auto* op_node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});
    auto* target = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});

    auto result = op_node->CanConnectTo(0, target, 10);

    EXPECT_FALSE(result.has_value());
}

TEST_F(OperatorNodeTest, AllOperatorTypes_CanBeCreatedAndUsed) {
    std::vector<core::OperatorNode::OperatorType> all_ops = {
        // Arithmetic
        core::OperatorNode::OperatorType::kAddition,
        core::OperatorNode::OperatorType::kSubtraction,
        core::OperatorNode::OperatorType::kMultiplication,
        core::OperatorNode::OperatorType::kDivision,
        core::OperatorNode::OperatorType::kModulo,
        // Bitwise
        core::OperatorNode::OperatorType::kBitwiseAnd,
        core::OperatorNode::OperatorType::kBitwiseOr,
        core::OperatorNode::OperatorType::kBitwiseXor,
        core::OperatorNode::OperatorType::kBitwiseNot,
        core::OperatorNode::OperatorType::kLeftShift,
        core::OperatorNode::OperatorType::kRightShift,
        // Comparison
        core::OperatorNode::OperatorType::kEqual,
        core::OperatorNode::OperatorType::kNotEqual,
        core::OperatorNode::OperatorType::kLessThan,
        core::OperatorNode::OperatorType::kGreaterThan,
        core::OperatorNode::OperatorType::kLessOrEqual,
        core::OperatorNode::OperatorType::kGreaterOrEqual,
        // Logical
        core::OperatorNode::OperatorType::kLogicalAnd,
        core::OperatorNode::OperatorType::kLogicalOr,
        core::OperatorNode::OperatorType::kLogicalNot
    };

    for (auto op_type : all_ops) {
        auto* node = graph_.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0 ,0});
        node->SetOperatorType(op_type);

        EXPECT_NE(node, nullptr);
        EXPECT_EQ(node->operator_type(), op_type);
        EXPECT_GE(node->GetInputPinCount(), 1);
        EXPECT_LE(node->GetInputPinCount(), 2);
        EXPECT_EQ(node->GetOutputPinCount(), 1);
        EXPECT_NE(node->GetCategory(), "");

        nlohmann::json json = node->Serialize();
        EXPECT_TRUE(json.contains("operator_type"));
    }
}
