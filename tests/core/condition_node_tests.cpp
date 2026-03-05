#include <gtest/gtest.h>

#include "graph.hpp"
#include "nodes/condition_node.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/variable_node.hpp"

class ConditionNodeTest : public testing::Test {
 protected:
    core::Graph graph_;
};

#pragma region Basic Properties

TEST_F(ConditionNodeTest, Constructor_DefaultValues_SetsCorrectDefaults) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kCondition);
    EXPECT_EQ(node->name(), "If/Else/Then");
    EXPECT_EQ(node->GetDisplayName(), "If/Else/Then");
    EXPECT_EQ(node->GetCategory(), "Branch");
}

TEST_F(ConditionNodeTest, SetName_ValidName_UpdatesName) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    node->set_name("MyCondition");
    
    EXPECT_EQ(node->name(), "MyCondition");
    EXPECT_EQ(node->GetDisplayName(), "MyCondition");
}

TEST_F(ConditionNodeTest, SetName_EmptyString_AcceptsEmptyName) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    node->set_name("");
    
    EXPECT_EQ(node->name(), "");
}

#pragma endregion

#pragma region Pin Configuration

TEST_F(ConditionNodeTest, GetInputPinCount_ReturnsOne) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    EXPECT_EQ(node->GetInputPinCount(), 1);
}

TEST_F(ConditionNodeTest, GetOutputPinCount_ReturnsOne) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    EXPECT_EQ(node->GetOutputPinCount(), 1);
}

TEST_F(ConditionNodeTest, GetInputPinType_Pin0_ReturnsBool) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    EXPECT_EQ(node->GetInputPinType(0), core::NodeBase::PinDataType::kBool);
}

TEST_F(ConditionNodeTest, GetOutputPinType_Pin0_ReturnsVoid) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    EXPECT_EQ(node->GetOutputPinType(0), core::NodeBase::PinDataType::kVoid);
}

TEST_F(ConditionNodeTest, GetOutputPinType_Pin1_ReturnsVoid) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    EXPECT_EQ(node->GetOutputPinType(1), core::NodeBase::PinDataType::kVoid);
}

TEST_F(ConditionNodeTest, GetOutputPinType_Pin2_ReturnsVoid) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    EXPECT_EQ(node->GetOutputPinType(2), core::NodeBase::PinDataType::kVoid);
}

TEST_F(ConditionNodeTest, GetOutputPinType_InvalidPin_ReturnsUndefined) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    EXPECT_EQ(node->GetOutputPinType(3), core::NodeBase::PinDataType::kUndefined);
    EXPECT_EQ(node->GetOutputPinType(10), core::NodeBase::PinDataType::kUndefined);
}

TEST_F(ConditionNodeTest, GetInputPinName_Pin0_ReturnsCondition) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    EXPECT_EQ(node->GetInputPinName(0), "Condition");
}

TEST_F(ConditionNodeTest, GetInputPinName_InvalidPin_ReturnsEmpty) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    EXPECT_EQ(node->GetInputPinName(1), "");
    EXPECT_EQ(node->GetInputPinName(5), "");
}

TEST_F(ConditionNodeTest, GetOutputPinName_ValidPins_ReturnsCorrectNames) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    EXPECT_EQ(node->GetOutputPinName(0), "If");
    EXPECT_EQ(node->GetOutputPinName(1), "Else");
    EXPECT_EQ(node->GetOutputPinName(2), "Then");
}

TEST_F(ConditionNodeTest, GetOutputPinName_InvalidPin_ReturnsEmpty) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    EXPECT_EQ(node->GetOutputPinName(3), "");
    EXPECT_EQ(node->GetOutputPinName(10), "");
}

#pragma endregion

#pragma region Connection Validation

TEST_F(ConditionNodeTest, CanConnectTo_ValidConnection_ReturnsSuccess) {
    auto* condition = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    auto* literal = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    
    // Set literal type to Bool to match condition input
    literal->set_type(core::NodeBase::PinDataType::kBool);
    
    auto result = literal->CanConnectTo(0, condition, 0);
    EXPECT_TRUE(result.has_value());
}

TEST_F(ConditionNodeTest, CanConnectTo_InvalidOutputPin_ReturnsError) {
    auto* condition = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    auto* target = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    auto result = condition->CanConnectTo(5, target, 0);
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(result.error().empty());
}

TEST_F(ConditionNodeTest, CanConnectTo_InvalidInputPin_ReturnsError) {
    auto* condition = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    auto* target = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    
    auto result = condition->CanConnectTo(0, target, 10);
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(result.error().empty());
}

TEST_F(ConditionNodeTest, CanConnectTo_TypeMismatch_ReturnsError) {
    auto* condition = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    auto* variable = graph_.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable);
    
    // Variable expects Int but condition outputs Void
    variable->set_type(core::NodeBase::PinDataType::kInt);
    
    auto result = condition->CanConnectTo(0, variable, 0);
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(result.error().empty());
}

#pragma endregion

#pragma region Graph Integration

TEST_F(ConditionNodeTest, Link_BoolLiteralToCondition_Success) {
    auto* literal = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto* condition = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    literal->set_type(core::NodeBase::PinDataType::kBool);
    
    auto result = graph_.Link(literal, 0, condition, 0);
    EXPECT_TRUE(result.has_value());
    
    // Verify connection was established
    auto* parent = condition->parent(0);
    ASSERT_NE(parent, nullptr);
    EXPECT_EQ(parent->node, literal);
    EXPECT_EQ(parent->pin, 0);
}

TEST_F(ConditionNodeTest, Link_ConditionToMultipleNodes_AllowsMultipleOutputs) {
    auto* condition = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    auto* target1 = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    auto* target2 = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    auto* target3 = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    // Connect If pin to multiple nodes
    auto result1 = graph_.Link(condition, 0, target1, 0);
    auto result2 = graph_.Link(condition, 1, target2, 0);
    auto result3 = graph_.Link(condition, 2, target3, 0);
    
    EXPECT_TRUE(result1.has_value());
    EXPECT_TRUE(result2.has_value());
    EXPECT_TRUE(result3.has_value());
}

TEST_F(ConditionNodeTest, Link_WrongTypeToCondition_Fails) {
    auto* literal = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto* condition = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    // Set Int type instead of Bool
    literal->set_type(core::NodeBase::PinDataType::kInt);
    
    auto result = graph_.Link(literal, 0, condition, 0);
    EXPECT_FALSE(result.has_value());
}

#pragma endregion

#pragma region Serialization

TEST_F(ConditionNodeTest, Serialize_DefaultNode_ProducesValidJson) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    auto json = node->Serialize();
    
    EXPECT_TRUE(json.contains("id"));
    EXPECT_TRUE(json.contains("kind"));
    EXPECT_TRUE(json.contains("name"));
    
    EXPECT_EQ(json["id"], node->id());
    EXPECT_EQ(json["kind"], "kCondition");
    EXPECT_EQ(json["name"], "If/Else/Then");
}

TEST_F(ConditionNodeTest, Serialize_CustomName_IncludesCustomName) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    node->set_name("CheckValue");
    
    auto json = node->Serialize();
    
    EXPECT_EQ(json["name"], "CheckValue");
}

TEST_F(ConditionNodeTest, Deserialize_ValidJson_RestoresState) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    nlohmann::json json;
    json["name"] = "DeserializedCondition";
    
    auto result = node->Deserialize(json);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(node->name(), "DeserializedCondition");
}

TEST_F(ConditionNodeTest, Deserialize_MissingName_ReturnsError) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    nlohmann::json json;
    // Intentionally missing "name" field
    
    auto result = node->Deserialize(json);
    
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(result.error().empty());
}

TEST_F(ConditionNodeTest, Deserialize_InvalidJsonType_ReturnsError) {
    auto* node = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    nlohmann::json json;
    json["name"] = 12345;  // Wrong type, should be string
    
    auto result = node->Deserialize(json);
    
    EXPECT_FALSE(result.has_value());
}

TEST_F(ConditionNodeTest, SerializeDeserialize_RoundTrip_PreservesData) {
    auto* original = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    original->set_name("TestCondition");
    
    auto json = original->Serialize();
    
    auto* restored = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    auto result = restored->Deserialize(json);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(restored->name(), original->name());
    EXPECT_EQ(restored->GetDisplayName(), original->GetDisplayName());
}

#pragma endregion

#pragma region Edge Cases

TEST_F(ConditionNodeTest, MultipleConditions_ChainedBranching_MaintainsConnections) {
    auto* bool_literal = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto* condition1 = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    auto* condition2 = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    auto* condition3 = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    bool_literal->set_type(core::NodeBase::PinDataType::kBool);
    condition1->set_name("FirstCheck");
    condition2->set_name("SecondCheck");
    condition3->set_name("ThirdCheck");
    
    // Create chain: literal -> condition1 (if) -> condition2 (else) -> condition3
    ASSERT_TRUE(graph_.Link(bool_literal, 0, condition1, 0).has_value());
    ASSERT_TRUE(graph_.Link(condition1, 0, condition2, 0).has_value());
    ASSERT_TRUE(graph_.Link(condition1, 1, condition3, 0).has_value());
    
    // Verify all connections
    EXPECT_NE(condition1->parent(0), nullptr);
    EXPECT_NE(condition2->parent(0), nullptr);
    EXPECT_NE(condition3->parent(0), nullptr);
    
    EXPECT_FALSE(condition1->childrens(0).empty());
    EXPECT_FALSE(condition1->childrens(1).empty());
}

TEST_F(ConditionNodeTest, RemoveCondition_WithConnections_CleansUpProperly) {
    auto* bool_literal = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto* condition = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    auto* target = graph_.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition);
    
    bool_literal->set_type(core::NodeBase::PinDataType::kBool);
    
    graph_.Link(bool_literal, 0, condition, 0);
    graph_.Link(condition, 0, target, 0);
    
    // Remove the middle condition node
    graph_.RemoveNode(condition);
    
    // Verify connections are cleaned up
    const auto& literal_children = bool_literal->childrens(0);
    for (const auto& conn : literal_children) {
        if (conn.IsConnected()) {
            EXPECT_NE(conn.node, condition);
        }
    }
    
    auto* target_parent = target->parent(0);
    if (target_parent && target_parent->IsConnected()) {
        EXPECT_NE(target_parent->node, condition);
    }
}

#pragma endregion
