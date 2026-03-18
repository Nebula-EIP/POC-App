#include <gtest/gtest.h>

#include "graph.hpp"
#include "nodes/function_input_node.hpp"
#include "nodes/function_node.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/variable_node.hpp"

namespace {

class FunctionNodeTest : public testing::Test {
   protected:
    core::Graph graph_;
};

// ---------- Creation ----------

TEST_F(FunctionNodeTest, AddNode_FunctionKind_ReturnsNonNull) {
    auto *node = graph_.AddNode(core::NodeBase::NodeKind::kFunction);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kFunction);
}

TEST_F(FunctionNodeTest, AddNode_Templated_ReturnsCorrectType) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kFunction);
    EXPECT_EQ(node->GetDisplayName(), "Function");
}

// ---------- Name ----------

TEST_F(FunctionNodeTest, Name_DefaultIsFunction) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    EXPECT_EQ(node->name(), "Function");
    EXPECT_EQ(node->GetDisplayName(), "Function");
}

TEST_F(FunctionNodeTest, Name_SetAndGet) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    node->set_name("my_add");
    EXPECT_EQ(node->name(), "my_add");
    EXPECT_EQ(node->GetDisplayName(), "my_add");
}

// ---------- Return type ----------

TEST_F(FunctionNodeTest, ReturnType_DefaultIsVoid) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    EXPECT_EQ(node->return_type(), core::NodeBase::PinDataType::kVoid);
}

TEST_F(FunctionNodeTest, ReturnType_SetAndGet) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    node->set_return_type(core::NodeBase::PinDataType::kInt);
    EXPECT_EQ(node->return_type(), core::NodeBase::PinDataType::kInt);
}

// ---------- Parameters (input pins) ----------

TEST_F(FunctionNodeTest, Parameters_DefaultEmpty) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    EXPECT_EQ(node->GetInputPinCount(), 0);
    EXPECT_TRUE(node->parameters().empty());
}

TEST_F(FunctionNodeTest, Parameters_AddOne_IncreasesInputPinCount) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    graph_.AddInputPin(node, "x", core::NodeBase::PinDataType::kInt);

    EXPECT_EQ(node->GetInputPinCount(), 1);
    EXPECT_EQ(node->parameters().size(), 1);
    EXPECT_EQ(node->parameters()[0].name, "x");
    EXPECT_EQ(node->parameters()[0].type, core::NodeBase::PinDataType::kInt);
}

TEST_F(FunctionNodeTest, Parameters_AddMultiple_CorrectPinCountAndTypes) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    graph_.AddInputPin(node, "a", core::NodeBase::PinDataType::kInt);
    graph_.AddInputPin(node, "b", core::NodeBase::PinDataType::kFloat);
    graph_.AddInputPin(node, "c", core::NodeBase::PinDataType::kBool);

    EXPECT_EQ(node->GetInputPinCount(), 3);

    EXPECT_EQ(node->GetInputPinType(0), core::NodeBase::PinDataType::kInt);
    EXPECT_EQ(node->GetInputPinType(1), core::NodeBase::PinDataType::kFloat);
    EXPECT_EQ(node->GetInputPinType(2), core::NodeBase::PinDataType::kBool);
}

TEST_F(FunctionNodeTest, Parameters_InputPinNames_MatchParameterNames) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    graph_.AddInputPin(node, "alpha", core::NodeBase::PinDataType::kInt);
    graph_.AddInputPin(node, "beta", core::NodeBase::PinDataType::kFloat);

    EXPECT_EQ(node->GetInputPinName(0), "alpha");
    EXPECT_EQ(node->GetInputPinName(1), "beta");
}

TEST_F(FunctionNodeTest, Parameters_RemoveOne_DecreasesPinCount) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    graph_.AddInputPin(node, "a", core::NodeBase::PinDataType::kInt);
    graph_.AddInputPin(node, "b", core::NodeBase::PinDataType::kFloat);
    EXPECT_EQ(node->GetInputPinCount(), 2);

    graph_.RemoveInputPin(node, static_cast<uint8_t>(0));
    EXPECT_EQ(node->GetInputPinCount(), 1);
    EXPECT_EQ(node->parameters()[0].name, "b");
}

TEST_F(FunctionNodeTest, Parameters_RemoveOutOfRange_DoesNotCrash) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    EXPECT_NO_THROW(graph_.RemoveInputPin(node, static_cast<uint8_t>(0)));
    EXPECT_NO_THROW(graph_.RemoveInputPin(node, static_cast<uint8_t>(255)));
}

// ---------- Output pin ----------

TEST_F(FunctionNodeTest, OutputPin_AlwaysOne) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    EXPECT_EQ(node->GetOutputPinCount(), 1);
}

TEST_F(FunctionNodeTest, OutputPin_TypeMatchesReturnType) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    node->set_return_type(core::NodeBase::PinDataType::kFloat);
    EXPECT_EQ(node->GetOutputPinType(0), core::NodeBase::PinDataType::kFloat);
}

TEST_F(FunctionNodeTest, OutputPin_NameIsResult) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    EXPECT_EQ(node->GetOutputPinName(0), "Result");
}

TEST_F(FunctionNodeTest, OutputPin_InvalidIndex_ReturnsUndefined) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    EXPECT_EQ(node->GetOutputPinType(1), core::NodeBase::PinDataType::kUndefined);
    EXPECT_EQ(node->GetOutputPinName(1), "");
}

TEST_F(FunctionNodeTest, InputPin_InvalidIndex_ReturnsUndefined) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    EXPECT_EQ(node->GetInputPinType(0), core::NodeBase::PinDataType::kUndefined);
    EXPECT_EQ(node->GetInputPinName(0), "");
}

// ---------- Category ----------

TEST_F(FunctionNodeTest, Category_ReturnsFunctions) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    EXPECT_EQ(node->GetCategory(), "Functions");
}

// ---------- Connection validation ----------

TEST_F(FunctionNodeTest, CanConnectTo_MatchingTypes_Succeeds) {
    auto *func_node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);
    func_node->set_return_type(core::NodeBase::PinDataType::kInt);

    auto *var_node = graph_.AddNode<core::VariableNode>(
        core::NodeBase::NodeKind::kVariable);
    var_node->set_type(core::NodeBase::PinDataType::kInt);

    auto result = func_node->CanConnectTo(0, var_node, 0);
    EXPECT_TRUE(result.has_value());
}

TEST_F(FunctionNodeTest, CanConnectTo_MismatchedTypes_Fails) {
    auto *func_node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);
    func_node->set_return_type(core::NodeBase::PinDataType::kInt);

    auto *var_node = graph_.AddNode<core::VariableNode>(
        core::NodeBase::NodeKind::kVariable);
    var_node->set_type(core::NodeBase::PinDataType::kFloat);

    auto result = func_node->CanConnectTo(0, var_node, 0);
    EXPECT_FALSE(result.has_value());
}

TEST_F(FunctionNodeTest, CanConnectTo_InvalidOutputPin_Fails) {
    auto *func_node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);
    auto *var_node = graph_.AddNode<core::VariableNode>(
        core::NodeBase::NodeKind::kVariable);

    auto result = func_node->CanConnectTo(1, var_node, 0);
    EXPECT_FALSE(result.has_value());
}

// ---------- Linking via Graph ----------

TEST_F(FunctionNodeTest, Link_LiteralToFunctionInput_Succeeds) {
    auto *literal = graph_.AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral);
    literal->set_type(core::NodeBase::PinDataType::kInt);

    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);
    graph_.AddInputPin(func, "x", core::NodeBase::PinDataType::kInt);
    func->set_return_type(core::NodeBase::PinDataType::kInt);

    EXPECT_NO_THROW({
        graph_.Link(literal, 0, func, 0);
    });

    auto parent = func->parent(0);
    EXPECT_EQ(parent.node, literal);
}

TEST_F(FunctionNodeTest, Link_FunctionOutputToVariable_Succeeds) {
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);
    func->set_return_type(core::NodeBase::PinDataType::kInt);

    auto *var = graph_.AddNode<core::VariableNode>(
        core::NodeBase::NodeKind::kVariable);
    var->set_type(core::NodeBase::PinDataType::kInt);

    EXPECT_NO_THROW({
        graph_.Link(func, 0, var, 0);
    });

    auto parent = var->parent(0);
    EXPECT_EQ(parent.node, func);
}

/**
TEST_F(FunctionNodeTest, Link_MultipleInputs_AllConnected) {
    auto *lit_a = graph_.AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral);
    lit_a->set_type(core::NodeBase::PinDataType::kInt);

    auto *lit_b = graph_.AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral);
    lit_b->set_type(core::NodeBase::PinDataType::kFloat);

    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);
    graph_.AddInputPin(func, "a", core::NodeBase::PinDataType::kInt);
    graph_.AddInputPin(func, "b", core::NodeBase::PinDataType::kFloat);
    func->set_return_type(core::NodeBase::PinDataType::kVoid);

    EXPECT_NO_THROW({
        graph_.Link(lit_a, 0, func, 0);
    });
    EXPECT_NO_THROW({
        graph_.Link(lit_b, 0, func, 1);
    });

    auto parent_a = func->parent(0);
    EXPECT_EQ(parent_a.node, lit_a);

    auto parent_b = func->parent(1);
    EXPECT_EQ(parent_b.node, lit_b);
}
    */

// ---------- Inner graph (body) ----------

TEST_F(FunctionNodeTest, Body_DefaultIsEmptyGraph) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    const core::Graph &body = node->body();

    // The inner graph should serialize with no nodes and no connections
    auto json = body.Serialize();
    EXPECT_EQ(json["graph"]["nodes"].size(), 0);
    EXPECT_EQ(json["graph"]["connections"].size(), 0);
}

TEST_F(FunctionNodeTest, Body_CanAddNodesInside) {
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    core::Graph &body = func->body();
    auto *inner_lit = body.AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral);
    inner_lit->set_type(core::NodeBase::PinDataType::kInt);

    auto *inner_var = body.AddNode<core::VariableNode>(
        core::NodeBase::NodeKind::kVariable);
    inner_var->set_type(core::NodeBase::PinDataType::kInt);

    EXPECT_NE(body.GetNode(inner_lit->id()), nullptr);
    EXPECT_NE(body.GetNode(inner_var->id()), nullptr);
}

TEST_F(FunctionNodeTest, Body_CanLinkNodesInside) {
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    core::Graph &body = func->body();
    auto *lit = body.AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral);
    lit->set_type(core::NodeBase::PinDataType::kInt);

    auto *var = body.AddNode<core::VariableNode>(
        core::NodeBase::NodeKind::kVariable);
    var->set_type(core::NodeBase::PinDataType::kInt);

    EXPECT_NO_THROW({
        body.Link(lit, 0, var, 0);
    });

    auto parent = var->parent(0);
    EXPECT_EQ(parent.node, lit);
}

TEST_F(FunctionNodeTest, Body_IsIndependentFromOuterGraph) {
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    core::Graph &body = func->body();
    body.AddNode(core::NodeBase::NodeKind::kLiteral);
    body.AddNode(core::NodeBase::NodeKind::kLiteral);

    // Inner graph has 2 nodes, outer graph has only the function node
    auto outer_json = graph_.Serialize();
    auto inner_json = body.Serialize();

    EXPECT_EQ(outer_json["graph"]["nodes"].size(), 1);
    EXPECT_EQ(inner_json["graph"]["nodes"].size(), 2);
}

// ---------- Serialization ----------

TEST_F(FunctionNodeTest, Serialize_ContainsAllFields) {
    auto *node = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);
    node->set_name("compute");
    node->set_return_type(core::NodeBase::PinDataType::kInt);
    graph_.AddInputPin(node, "x", core::NodeBase::PinDataType::kInt);
    graph_.AddInputPin(node, "y", core::NodeBase::PinDataType::kFloat);

    auto json = node->Serialize();

    EXPECT_EQ(json["kind"], "Function");
    EXPECT_EQ(json["name"], "compute");
    EXPECT_EQ(json["return_type"], "Int");

    ASSERT_TRUE(json.contains("parameters"));
    ASSERT_EQ(json["parameters"].size(), 2);
    EXPECT_EQ(json["parameters"][0]["name"], "x");
    EXPECT_EQ(json["parameters"][0]["type"], "Int");
    EXPECT_EQ(json["parameters"][1]["name"], "y");
    EXPECT_EQ(json["parameters"][1]["type"], "Float");

    EXPECT_TRUE(json.contains("body"));
    EXPECT_TRUE(json["body"].contains("graph"));
}

TEST_F(FunctionNodeTest, Serialize_BodyIncludesInnerNodes) {
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    auto *inner = func->body().AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral);
    inner->set_type(core::NodeBase::PinDataType::kInt);
    inner->set_name("Constant42");

    auto json = func->Serialize();

    ASSERT_EQ(json["body"]["graph"]["nodes"].size(), 1);
    EXPECT_EQ(json["body"]["graph"]["nodes"][0]["name"], "Constant42");
}

// ---------- Deserialization ----------

/*
TEST_F(FunctionNodeTest, Deserialize_RoundTrip_PreservesData) {
    // Build a function node with parameters and inner graph
    auto *original = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);
    original->set_name("my_func");
    original->set_return_type(core::NodeBase::PinDataType::kFloat);
    graph_.AddInputPin(original, "a", core::NodeBase::PinDataType::kInt);
    graph_.AddInputPin(original, "b", core::NodeBase::PinDataType::kString);

    auto *inner_lit = original->body().AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral);
    inner_lit->set_type(core::NodeBase::PinDataType::kFloat);
    inner_lit->set_name("pi");

    // Serialize the entire graph
    auto json = graph_.Serialize();

    // Deserialize into a new graph
    auto result = core::Graph::Deserialize(json);
    ASSERT_TRUE(result.has_value()) << result.error();

    auto &restored_graph = result.value();
    auto *restored = restored_graph.GetNode<core::FunctionNode>(original->id());
    ASSERT_NE(restored, nullptr);

    EXPECT_EQ(restored->name(), "my_func");
    EXPECT_EQ(restored->return_type(), core::NodeBase::PinDataType::kFloat);

    ASSERT_EQ(restored->parameters().size(), 2);
    EXPECT_EQ(restored->parameters()[0].name, "a");
    EXPECT_EQ(restored->parameters()[0].type, core::NodeBase::PinDataType::kInt);
    EXPECT_EQ(restored->parameters()[1].name, "b");
    EXPECT_EQ(restored->parameters()[1].type,
              core::NodeBase::PinDataType::kString);

    // Check inner graph was restored (2 FunctionInputNodes + 1 LiteralNode)
    auto inner_json = restored->body().Serialize();
    EXPECT_EQ(inner_json["graph"]["nodes"].size(), 3);
    // Find the literal node among the restored body nodes
    bool found_pi = false;
    for (const auto &n : inner_json["graph"]["nodes"]) {
        if (n["name"] == "pi") {
            found_pi = true;
            break;
        }
    }
    EXPECT_TRUE(found_pi);
}
    */

TEST_F(FunctionNodeTest, Deserialize_MissingFields_ReturnsError) {
    nlohmann::json bad_json;
    bad_json["id"] = 0;
    bad_json["kind"] = "Function";
    // Missing name, return_type, parameters

    nlohmann::json full;
    full["metadata"]["project_name"] = "test";
    full["metadata"]["version"] = "1.0";
    full["graph"]["next_id"] = 1;
    full["graph"]["nodes"] = nlohmann::json::array({bad_json});
    full["graph"]["connections"] = nlohmann::json::array();

    auto result = core::Graph::Deserialize(full);
    EXPECT_FALSE(result.has_value());
}

TEST_F(FunctionNodeTest, Deserialize_EmptyParameters_ProducesZeroInputs) {
    nlohmann::json node_json;
    node_json["id"] = 0;
    node_json["kind"] = "Function";
    node_json["name"] = "noop";
    node_json["return_type"] = "Void";
    node_json["parameters"] = nlohmann::json::array();
    // Minimal body with empty graph
    node_json["body"]["metadata"]["project_name"] = "inner";
    node_json["body"]["metadata"]["version"] = "1.0";
    node_json["body"]["graph"]["next_id"] = 0;
    node_json["body"]["graph"]["nodes"] = nlohmann::json::array();
    node_json["body"]["graph"]["connections"] = nlohmann::json::array();

    nlohmann::json full;
    full["metadata"]["project_name"] = "test";
    full["metadata"]["version"] = "1.0";
    full["graph"]["next_id"] = 1;
    full["graph"]["nodes"] = nlohmann::json::array({node_json});
    full["graph"]["connections"] = nlohmann::json::array();

    auto result = core::Graph::Deserialize(full);
    ASSERT_TRUE(result.has_value()) << result.error();

    auto *node = result.value().GetNode<core::FunctionNode>(0);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetInputPinCount(), 0);
    EXPECT_EQ(node->GetOutputPinCount(), 1);
    EXPECT_EQ(node->return_type(), core::NodeBase::PinDataType::kVoid);
}

// ---------- File round-trip ----------

/**
TEST_F(FunctionNodeTest, SaveAndLoad_PreservesFunctionNode) {
    namespace fs = std::filesystem;
    fs::path tmp = fs::temp_directory_path() / "nebula_func_test.nebula";

    graph_.SetProjectName("FuncTest");
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);
    func->set_name("add");
    func->set_return_type(core::NodeBase::PinDataType::kInt);
    graph_.AddInputPin(func, "lhs", core::NodeBase::PinDataType::kInt);
    graph_.AddInputPin(func, "rhs", core::NodeBase::PinDataType::kInt);

    auto *inner = func->body().AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral);
    inner->set_type(core::NodeBase::PinDataType::kInt);
    inner->set_name("result");

    auto save_result = graph_.SaveToFile(tmp);
    ASSERT_TRUE(save_result.has_value()) << save_result.error();

    auto load_result = core::Graph::LoadFromFile(tmp);
    ASSERT_TRUE(load_result.has_value()) << load_result.error();

    auto &loaded = load_result.value();
    EXPECT_EQ(loaded.GetProjectName(), "FuncTest");

    auto *loaded_func = loaded.GetNode<core::FunctionNode>(func->id());
    ASSERT_NE(loaded_func, nullptr);
    EXPECT_EQ(loaded_func->name(), "add");
    EXPECT_EQ(loaded_func->return_type(), core::NodeBase::PinDataType::kInt);
    ASSERT_EQ(loaded_func->parameters().size(), 2);
    EXPECT_EQ(loaded_func->parameters()[0].name, "lhs");
    EXPECT_EQ(loaded_func->parameters()[1].name, "rhs");

    // 2 FunctionInputNodes (lhs, rhs) + 1 LiteralNode (result)
    auto body_json = loaded_func->body().Serialize();
    EXPECT_EQ(body_json["graph"]["nodes"].size(), 3);

    fs::remove(tmp);
}
    */

// ---------- AddParameter creates FunctionInputNode in body ----------

TEST_F(FunctionNodeTest, AddParameter_CreatesFunctionInputNodeInBody) {
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    graph_.AddInputPin(func, "x", core::NodeBase::PinDataType::kInt);

    const auto &params = func->parameters();
    ASSERT_EQ(params.size(), 1);

    auto *input_node =
        func->body().GetNode<core::FunctionInputNode>(params[0].node_id);
    ASSERT_NE(input_node, nullptr);
    EXPECT_EQ(input_node->name(), "x");
    EXPECT_EQ(input_node->type(), core::NodeBase::PinDataType::kInt);
}

TEST_F(FunctionNodeTest, AddParameter_Multiple_CreatesOneNodePerParam) {
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    graph_.AddInputPin(func, "a", core::NodeBase::PinDataType::kInt);
    graph_.AddInputPin(func, "b", core::NodeBase::PinDataType::kFloat);
    graph_.AddInputPin(func, "c", core::NodeBase::PinDataType::kBool);

    const auto &params = func->parameters();
    ASSERT_EQ(params.size(), 3);

    for (const auto &param : params) {
        auto *input_node =
            func->body().GetNode<core::FunctionInputNode>(param.node_id);
        ASSERT_NE(input_node, nullptr) << "Missing node for param: " << param.name;
        EXPECT_EQ(input_node->name(), param.name);
        EXPECT_EQ(input_node->type(), param.type);
    }

    // Body should contain exactly 3 nodes
    auto body_json = func->body().Serialize();
    EXPECT_EQ(body_json["graph"]["nodes"].size(), 3);
}

// ---------- RemoveParameter(index) removes node from body ----------

TEST_F(FunctionNodeTest, RemoveParameterByIndex_RemovesNodeFromBody) {
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    graph_.AddInputPin(func, "a", core::NodeBase::PinDataType::kInt);
    graph_.AddInputPin(func, "b", core::NodeBase::PinDataType::kFloat);

    uint32_t removed_node_id = func->parameters()[0].node_id;
    uint32_t kept_node_id = func->parameters()[1].node_id;

    graph_.RemoveInputPin(func, static_cast<uint8_t>(0));

    // The removed node should no longer exist in the body
    EXPECT_EQ(func->body().GetNode(removed_node_id), nullptr);

    // The kept node should still exist
    auto *kept = func->body().GetNode<core::FunctionInputNode>(kept_node_id);
    ASSERT_NE(kept, nullptr);
    EXPECT_EQ(kept->name(), "b");

    // Body should have exactly 1 node left
    auto body_json = func->body().Serialize();
    EXPECT_EQ(body_json["graph"]["nodes"].size(), 1);
}

TEST_F(FunctionNodeTest, RemoveParameterByIndex_LastParam_LeavesEmptyBody) {
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    graph_.AddInputPin(func, "x", core::NodeBase::PinDataType::kInt);
    uint32_t node_id = func->parameters()[0].node_id;

    graph_.RemoveInputPin(func, static_cast<uint8_t>(0));

    EXPECT_EQ(func->body().GetNode(node_id), nullptr);
    EXPECT_TRUE(func->parameters().empty());

    auto body_json = func->body().Serialize();
    EXPECT_EQ(body_json["graph"]["nodes"].size(), 0);
}

// ---------- RemoveParameter(name) removes node from body ----------

TEST_F(FunctionNodeTest, RemoveParameterByName_RemovesNodeFromBody) {
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    graph_.AddInputPin(func, "a", core::NodeBase::PinDataType::kInt);
    graph_.AddInputPin(func, "b", core::NodeBase::PinDataType::kFloat);
    graph_.AddInputPin(func, "c", core::NodeBase::PinDataType::kBool);

    uint32_t removed_id = func->parameters()[1].node_id;  // "b"

    graph_.RemoveInputPin(func, std::string("b"));

    // "b" node removed from body
    EXPECT_EQ(func->body().GetNode(removed_id), nullptr);

    // Remaining params are "a" and "c"
    ASSERT_EQ(func->parameters().size(), 2);
    EXPECT_EQ(func->parameters()[0].name, "a");
    EXPECT_EQ(func->parameters()[1].name, "c");

    // Both remaining nodes still exist
    for (const auto &param : func->parameters()) {
        EXPECT_NE(func->body().GetNode(param.node_id), nullptr)
            << "Node for " << param.name << " should still exist";
    }

    auto body_json = func->body().Serialize();
    EXPECT_EQ(body_json["graph"]["nodes"].size(), 2);
}

TEST_F(FunctionNodeTest, RemoveParameterByName_NonExistent_DoesNothing) {
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    graph_.AddInputPin(func, "x", core::NodeBase::PinDataType::kInt);

    graph_.RemoveInputPin(func, std::string("does_not_exist"));

    // Nothing changed
    ASSERT_EQ(func->parameters().size(), 1);
    EXPECT_EQ(func->parameters()[0].name, "x");

    auto body_json = func->body().Serialize();
    EXPECT_EQ(body_json["graph"]["nodes"].size(), 1);
}

// ---------- Body nodes survive alongside manually added nodes ----------

TEST_F(FunctionNodeTest, AddParameter_CoexistsWithManualBodyNodes) {
    auto *func = graph_.AddNode<core::FunctionNode>(
        core::NodeBase::NodeKind::kFunction);

    // Manually add a literal node to the body
    auto *lit = func->body().AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral);
    lit->set_type(core::NodeBase::PinDataType::kInt);

    // Now add a parameter — creates another node
    graph_.AddInputPin(func, "x", core::NodeBase::PinDataType::kInt);

    auto body_json = func->body().Serialize();
    EXPECT_EQ(body_json["graph"]["nodes"].size(), 2);

    // Remove the parameter — only its node is removed, literal stays
    graph_.RemoveInputPin(func, static_cast<uint8_t>(0));

    body_json = func->body().Serialize();
    EXPECT_EQ(body_json["graph"]["nodes"].size(), 1);
    EXPECT_NE(func->body().GetNode(lit->id()), nullptr);
}

}  // namespace
