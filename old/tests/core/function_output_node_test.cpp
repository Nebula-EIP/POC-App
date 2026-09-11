#include <gtest/gtest.h>

#include "graph.hpp"
#include "nodes/function_node.hpp"
#include "nodes/function_output_node.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/variable_node.hpp"

namespace {

class FunctionOutputNodeTest : public testing::Test {
   protected:
    core::Graph graph_;
};

// ---------- Creation ----------

TEST_F(FunctionOutputNodeTest, AddNode_ReturnsNonNull) {
    auto *node = graph_.AddNode(core::NodeBase::NodeKind::kFunctionOutput, {0, 0});
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kFunctionOutput);
}

TEST_F(FunctionOutputNodeTest, AddNode_Templated_ReturnsCorrectType) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kFunctionOutput);
}

// ---------- Name ----------

TEST_F(FunctionOutputNodeTest, Name_DefaultIsOutput) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});

    EXPECT_EQ(node->Name(), "Output");
    EXPECT_EQ(node->GetDisplayName(), "Output");
}

TEST_F(FunctionOutputNodeTest, Name_SetAndGet) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});

    node->SetName("Return");
    EXPECT_EQ(node->Name(), "Return");
    EXPECT_EQ(node->GetDisplayName(), "Return");
}

// ---------- Type ----------

TEST_F(FunctionOutputNodeTest, Type_DefaultIsUndefined) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});

    EXPECT_EQ(node->type(), core::NodeBase::PinDataType::kUndefined);
}

TEST_F(FunctionOutputNodeTest, Type_SetAndGet) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});

    node->SetType(core::NodeBase::PinDataType::kInt);
    EXPECT_EQ(node->type(), core::NodeBase::PinDataType::kInt);
}

// ---------- Pin counts ----------

TEST_F(FunctionOutputNodeTest, InputPinCount_IsOne) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});

    EXPECT_EQ(node->GetInputPinCount(), 1);
}

TEST_F(FunctionOutputNodeTest, OutputPinCount_IsZero) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});

    EXPECT_EQ(node->GetOutputPinCount(), 0);
}

// ---------- Pin types ----------

TEST_F(FunctionOutputNodeTest, InputPinType_MatchesNodeType) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});
    node->SetType(core::NodeBase::PinDataType::kFloat);

    EXPECT_EQ(node->GetInputPinType(0), core::NodeBase::PinDataType::kFloat);
}

TEST_F(FunctionOutputNodeTest, InputPinType_InvalidIndex_ReturnsUndefined) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});
    node->SetType(core::NodeBase::PinDataType::kInt);

    EXPECT_EQ(node->GetInputPinType(1),
              core::NodeBase::PinDataType::kUndefined);
}

TEST_F(FunctionOutputNodeTest, OutputPinType_AlwaysUndefined) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});
    node->SetType(core::NodeBase::PinDataType::kInt);

    EXPECT_EQ(node->GetOutputPinType(0),
              core::NodeBase::PinDataType::kUndefined);
}

// ---------- Pin names ----------

TEST_F(FunctionOutputNodeTest, InputPinName_MatchesNodeName) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});
    node->SetName("Result");

    EXPECT_EQ(node->GetInputPinName(0), "Result");
}

TEST_F(FunctionOutputNodeTest, InputPinName_InvalidIndex_ReturnsEmpty) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});

    EXPECT_EQ(node->GetInputPinName(1), "");
}

TEST_F(FunctionOutputNodeTest, OutputPinName_AlwaysEmpty) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});

    EXPECT_EQ(node->GetOutputPinName(0), "");
}

// ---------- Category ----------

TEST_F(FunctionOutputNodeTest, Category_ReturnsFunctions) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});

    EXPECT_EQ(node->GetCategory(), "Functions");
}

// ---------- Connection validation ----------

TEST_F(FunctionOutputNodeTest, CanConnectTo_AlwaysFails) {
    auto *output_node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});
    output_node->SetType(core::NodeBase::PinDataType::kInt);

    auto *var_node = graph_.AddNode<core::VariableNode>(
        core::NodeBase::NodeKind::kVariable, {0, 0});
    var_node->SetType(core::NodeBase::PinDataType::kInt);

    auto result = output_node->CanConnectTo(0, var_node, 0);
    EXPECT_FALSE(result.has_value());
}

// ---------- Linking via Graph ----------

TEST_F(FunctionOutputNodeTest, Link_LiteralToOutput_Succeeds) {
    auto *literal = graph_.AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral, {0, 0});
    literal->SetType(core::NodeBase::PinDataType::kInt);

    auto *output_node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});
    output_node->SetType(core::NodeBase::PinDataType::kInt);

    EXPECT_NO_THROW({
        graph_.Link(literal, 0, output_node, 0);
    });

    auto parent = output_node->parent(0);
    EXPECT_EQ(parent.node, literal);
}

// ---------- Serialization ----------

TEST_F(FunctionOutputNodeTest, Serialize_ContainsAllFields) {
    auto *node = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});
    node->SetName("Return");
    node->SetType(core::NodeBase::PinDataType::kInt);

    auto json = node->Serialize();

    EXPECT_EQ(json["kind"], "FunctionOutput");
    EXPECT_EQ(json["name"], "Return");
    EXPECT_EQ(json["type"], "Int");
    EXPECT_TRUE(json.contains("id"));
}

// ---------- Deserialization ----------
/*
TEST_F(FunctionOutputNodeTest, Deserialize_RoundTrip_PreservesData) {
    auto *original = graph_.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput);
    original->SetName("ReturnVal");
    original->SetType(core::NodeBase::PinDataType::kFloat);

    auto json = graph_.Serialize();

    auto result = core::Graph::Deserialize(json);
    ASSERT_TRUE(result.has_value()) << result.error();

    auto &restored = result.value();
    auto *node =
        restored.GetNode<core::FunctionOutputNode>(original->id());
    ASSERT_NE(node, nullptr);

    EXPECT_EQ(node->Name(), "ReturnVal");
    EXPECT_EQ(node->type(), core::NodeBase::PinDataType::kFloat);
    EXPECT_EQ(node->GetInputPinCount(), 1);
    EXPECT_EQ(node->GetOutputPinCount(), 0);
}
*/

TEST_F(FunctionOutputNodeTest, Deserialize_MissingFields_ReturnsError) {
    nlohmann::json bad_json;
    bad_json["id"] = 0;
    bad_json["kind"] = "FunctionOutput";
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

// ---------- Usage inside function body ----------

TEST_F(FunctionOutputNodeTest, UsedInsideFunctionBody_CanReceiveInput) {
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction, {0, 0});
    func->set_return_type(core::NodeBase::PinDataType::kInt);

    core::Graph &body = func->body();

    auto *lit = body.AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral, {0, 0});
    lit->SetType(core::NodeBase::PinDataType::kInt);

    auto *out = body.AddNode<core::FunctionOutputNode>(
        core::NodeBase::NodeKind::kFunctionOutput, {0, 0});
    out->SetType(core::NodeBase::PinDataType::kInt);
    out->SetName("Return");

    EXPECT_NO_THROW({
        body.Link(lit, 0, out, 0);
    });

    auto parent = out->parent(0);
    EXPECT_EQ(parent.node, lit);
}

}  // namespace
