#include <gtest/gtest.h>

#include "graph.hpp"
#include "nodes/function_input_node.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/variable_node.hpp"

namespace {

class FunctionInputNodeTest : public testing::Test {
   protected:
    core::Graph graph_;
};

// ---------- Creation ----------

TEST_F(FunctionInputNodeTest, AddNode_ReturnsNonNull) {
    auto *node = graph_.AddNode(core::NodeBase::NodeKind::kFunctionInput);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kFunctionInput);
}

TEST_F(FunctionInputNodeTest, AddNode_Templated_ReturnsCorrectType) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kFunctionInput);
}

// ---------- Name ----------

TEST_F(FunctionInputNodeTest, Name_DefaultIsInput) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);

    EXPECT_EQ(node->name(), "Input");
    EXPECT_EQ(node->GetDisplayName(), "Input");
}

TEST_F(FunctionInputNodeTest, Name_SetAndGet) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);

    node->set_name("x");
    EXPECT_EQ(node->name(), "x");
    EXPECT_EQ(node->GetDisplayName(), "x");
}

// ---------- Type ----------

TEST_F(FunctionInputNodeTest, Type_DefaultIsUndefined) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);

    EXPECT_EQ(node->type(), core::NodeBase::PinDataType::kUndefined);
}

TEST_F(FunctionInputNodeTest, Type_SetAndGet) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);

    node->set_type(core::NodeBase::PinDataType::kInt);
    EXPECT_EQ(node->type(), core::NodeBase::PinDataType::kInt);
}

// ---------- Pin counts ----------

TEST_F(FunctionInputNodeTest, InputPinCount_IsZero) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);

    EXPECT_EQ(node->GetInputPinCount(), 0);
}

TEST_F(FunctionInputNodeTest, OutputPinCount_IsOne) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);

    EXPECT_EQ(node->GetOutputPinCount(), 1);
}

// ---------- Pin types ----------

TEST_F(FunctionInputNodeTest, InputPinType_AlwaysUndefined) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);
    node->set_type(core::NodeBase::PinDataType::kInt);

    EXPECT_EQ(node->GetInputPinType(0), core::NodeBase::PinDataType::kUndefined);
}

TEST_F(FunctionInputNodeTest, OutputPinType_MatchesNodeType) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);
    node->set_type(core::NodeBase::PinDataType::kFloat);

    EXPECT_EQ(node->GetOutputPinType(0), core::NodeBase::PinDataType::kFloat);
}

TEST_F(FunctionInputNodeTest, OutputPinType_InvalidIndex_ReturnsUndefined) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);
    node->set_type(core::NodeBase::PinDataType::kInt);

    EXPECT_EQ(node->GetOutputPinType(1),
              core::NodeBase::PinDataType::kUndefined);
}

// ---------- Pin names ----------

TEST_F(FunctionInputNodeTest, InputPinName_AlwaysEmpty) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);

    EXPECT_EQ(node->GetInputPinName(0), "");
}

TEST_F(FunctionInputNodeTest, OutputPinName_MatchesNodeName) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);
    node->set_name("param_x");

    EXPECT_EQ(node->GetOutputPinName(0), "param_x");
}

TEST_F(FunctionInputNodeTest, OutputPinName_InvalidIndex_ReturnsEmpty) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);

    EXPECT_EQ(node->GetOutputPinName(1), "");
}

// ---------- Category ----------

TEST_F(FunctionInputNodeTest, Category_ReturnsFunctions) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);

    EXPECT_EQ(node->GetCategory(), "Functions");
}

// ---------- Connection validation ----------

TEST_F(FunctionInputNodeTest, CanConnectTo_MatchingTypes_Succeeds) {
    auto *input_node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);
    input_node->set_type(core::NodeBase::PinDataType::kInt);

    auto *var_node = graph_.AddNode<core::VariableNode>(
        core::NodeBase::NodeKind::kVariable);
    var_node->set_type(core::NodeBase::PinDataType::kInt);

    auto result = input_node->CanConnectTo(0, var_node, 0);
    EXPECT_TRUE(result.has_value());
}

TEST_F(FunctionInputNodeTest, CanConnectTo_MismatchedTypes_Fails) {
    auto *input_node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);
    input_node->set_type(core::NodeBase::PinDataType::kInt);

    auto *var_node = graph_.AddNode<core::VariableNode>(
        core::NodeBase::NodeKind::kVariable);
    var_node->set_type(core::NodeBase::PinDataType::kFloat);

    auto result = input_node->CanConnectTo(0, var_node, 0);
    EXPECT_FALSE(result.has_value());
}

TEST_F(FunctionInputNodeTest, CanConnectTo_InvalidOutputPin_Fails) {
    auto *input_node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);
    auto *var_node = graph_.AddNode<core::VariableNode>(
        core::NodeBase::NodeKind::kVariable);

    auto result = input_node->CanConnectTo(1, var_node, 0);
    EXPECT_FALSE(result.has_value());
}

TEST_F(FunctionInputNodeTest, CanConnectTo_InvalidTargetPin_Fails) {
    auto *input_node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);
    input_node->set_type(core::NodeBase::PinDataType::kInt);

    auto *var_node = graph_.AddNode<core::VariableNode>(
        core::NodeBase::NodeKind::kVariable);
    var_node->set_type(core::NodeBase::PinDataType::kInt);

    auto result = input_node->CanConnectTo(0, var_node, 5);
    EXPECT_FALSE(result.has_value());
}

// ---------- Linking via Graph ----------

TEST_F(FunctionInputNodeTest, Link_ToVariable_Succeeds) {
    auto *input_node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);
    input_node->set_type(core::NodeBase::PinDataType::kInt);

    auto *var_node = graph_.AddNode<core::VariableNode>(
        core::NodeBase::NodeKind::kVariable);
    var_node->set_type(core::NodeBase::PinDataType::kInt);

    // Link now throws on error instead of returning expected
    EXPECT_NO_THROW({
        graph_.Link(input_node, 0, var_node, 0);
    });

    // Verify connection was established
    EXPECT_TRUE(var_node->IsInputPinConnected(0));
    auto parent = var_node->parent(0);
    EXPECT_EQ(parent.node, input_node);
}

// ---------- Serialization ----------

TEST_F(FunctionInputNodeTest, Serialize_ContainsAllFields) {
    auto *node = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);
    node->set_name("my_param");
    node->set_type(core::NodeBase::PinDataType::kFloat);

    auto json = node->Serialize();

    EXPECT_EQ(json["kind"], "FunctionInput");
    EXPECT_EQ(json["name"], "my_param");
    EXPECT_EQ(json["type"], "Float");
    EXPECT_TRUE(json.contains("id"));
}

// ---------- Deserialization ----------

/*
TEST_F(FunctionInputNodeTest, Deserialize_RoundTrip_PreservesData) {
    auto *original = graph_.AddNode<core::FunctionInputNode>(
        core::NodeBase::NodeKind::kFunctionInput);
    original->set_name("x_coord");
    original->set_type(core::NodeBase::PinDataType::kFloat);

    auto json = graph_.Serialize();

    auto result = core::Graph::Deserialize(json);
    ASSERT_TRUE(result.has_value()) << result.error();

    auto &restored = result.value();
    auto *node =
        restored.GetNode<core::FunctionInputNode>(original->id());
    ASSERT_NE(node, nullptr);

    EXPECT_EQ(node->name(), "x_coord");
    EXPECT_EQ(node->type(), core::NodeBase::PinDataType::kFloat);
    EXPECT_EQ(node->GetInputPinCount(), 0);
    EXPECT_EQ(node->GetOutputPinCount(), 1);
}

TEST_F(FunctionInputNodeTest, Deserialize_MissingFields_ReturnsError) {
    nlohmann::json bad_json;
    bad_json["id"] = 0;
    bad_json["kind"] = "FunctionInput";
    // Missing type and name

    nlohmann::json full;
    full["metadata"]["project_name"] = "test";
    full["metadata"]["version"] = "1.0";
    full["graph"]["next_id"] = 1;
    full["graph"]["nodes"] = nlohmann::json::array({bad_json});
    full["graph"]["connections"] = nlohmann::json::array();

    auto result = core::Graph::Deserialize(full);
    EXPECT_FALSE(result.has_value());
}
    */

}  // namespace
