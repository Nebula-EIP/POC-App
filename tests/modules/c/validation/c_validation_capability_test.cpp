/**
 * @file c_validation_capability_test.cpp
 * @brief Unit tests for CValidationCapability pre-codegen graph checks.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 21-11-2026
 */

#include "validation/c_validation_capability.hpp"

#include <gtest/gtest.h>

#include <any>
#include <string>
#include <string_view>
#include <vector>

#include "graph/graph.hpp"
#include "modules/capabilities/type_list_capability.hpp"
#include "nodes/c_node_list_capability.hpp"

namespace {

const std::vector<std::string_view> kCTypes = {
    "void", "int", "float", "bool", "char", "string",
};

class CValidationCapabilityTest : public ::testing::Test {
   protected:
    void SetUp() override {
        types_capa_ = new core::capa::TypeListCapability(kCTypes);
        for (std::size_t i = 0; i < kCTypes.size(); ++i) {
            types_capa_->RegisterType(static_cast<core::DataType>(i + 1));
        }

        node_capa_ = new c_module::CNodeListCapability(types_capa_);
        for (int i = 0; i < 10; ++i) {
            node_capa_->RegisterNode(static_cast<core::NodeType>(i + 1));
        }

        validator_ = new c_module::CValidationCapability();
        std::unordered_map<core::DataType, std::string> registered_types;
        for (const auto &type_def : types_capa_->Types()) {
            registered_types[type_def.id_] = std::string(type_def.name_);
        }
        std::unordered_map<core::NodeType, std::string> registered_node_types;
        for (const auto &node_meta : node_capa_->GetAvailableNodes()) {
            registered_node_types[node_meta.type_] = node_meta.name_;
        }
        validator_->InitializeTypes(registered_types);
        validator_->InitializeNodeTypes(registered_node_types);
    }

    void TearDown() override {
        delete validator_;
        delete node_capa_;
        delete types_capa_;
    }

    core::Node &SpawnCNode(core::Graph &graph, core::NodeType type) {
        core::Node &node = graph.CreateNode(type);
        const auto kConfig = node_capa_->GetNodeConfiguration(type);
        for (const auto &pin : kConfig.input_pins_) {
            graph.AddInputPin(node.Id(), pin.name_, pin.type_);
        }
        for (const auto &pin : kConfig.output_pins_) {
            graph.AddOutputPin(node.Id(), pin.name_, pin.type_);
        }
        for (const auto &[kPropId, kProp] : kConfig.default_properties_) {
            node.SetProperty(kPropId, kProp);
        }
        return node;
    }

    core::capa::TypeListCapability *types_capa_ = nullptr;
    c_module::CNodeListCapability *node_capa_ = nullptr;
    c_module::CValidationCapability *validator_ = nullptr;
};

TEST_F(CValidationCapabilityTest, EmptyGraphIsValid) {
    core::Graph graph;

    const auto kResult = validator_->Validate(graph);
    EXPECT_TRUE(kResult.is_valid_);
    EXPECT_EQ(kResult.ErrorCount(), 0U);
}

TEST_F(CValidationCapabilityTest, ValidConnectedGraphPassesConsistently) {
    core::Graph graph;

    // Literal 1 (int 10) + Literal 2 (int 20) -> Variable -> Print (int)
    core::Node &lit1 = SpawnCNode(graph, 2);
    lit1.SetProperty(c_module::properties::kValue,
                     {c_module::properties::kValue, 10.0});

    core::Node &lit2 = SpawnCNode(graph, 2);
    lit2.SetProperty(c_module::properties::kValue,
                     {c_module::properties::kValue, 20.0});

    core::Node &op = SpawnCNode(graph, 3);
    core::Node &var = SpawnCNode(graph, 1);

    core::Node &print_node = SpawnCNode(graph, 9);
    print_node.SetProperty(c_module::properties::kType,
                           {c_module::properties::kType, std::string("int")});
    graph.SetPinType(print_node.Id(), print_node.InputPins()[1].id_, true,
                     types_capa_->TypeId("int"));

    graph.Connect(lit1.Id(), lit1.OutputPins()[0].id_, op.Id(),
                  op.InputPins()[0].id_);
    graph.Connect(lit2.Id(), lit2.OutputPins()[0].id_, op.Id(),
                  op.InputPins()[1].id_);
    graph.Connect(op.Id(), op.OutputPins()[0].id_, var.Id(),
                  var.InputPins()[0].id_);
    graph.Connect(var.Id(), var.OutputPins()[0].id_, print_node.Id(),
                  print_node.InputPins()[1].id_);

    const auto kFirst = validator_->Validate(graph);
    const auto kSecond = validator_->Validate(graph);
    EXPECT_TRUE(kFirst.is_valid_);
    EXPECT_TRUE(kSecond.is_valid_);
    EXPECT_EQ(kFirst.ErrorCount(), 0U);
    EXPECT_EQ(kSecond.ErrorCount(), 0U);
}

TEST_F(CValidationCapabilityTest, DetectsMissingRequiredConnections) {
    core::Graph graph;

    // Operator requires both 'left' and 'right' inputs connected
    core::Node &lit = SpawnCNode(graph, 2);
    core::Node &op = SpawnCNode(graph, 3);
    graph.Connect(lit.Id(), lit.OutputPins()[0].id_, op.Id(),
                  op.InputPins()[0].id_);

    const auto kResult = validator_->Validate(graph);
    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasPinConnectivityError());
    ASSERT_EQ(kResult.ErrorCount(), 1U);
    EXPECT_EQ(kResult.errors_[0].involved_node_ids_[0], op.Id());
    ASSERT_EQ(kResult.errors_[0].involved_pins_.size(), 1U);
    EXPECT_EQ(kResult.errors_[0].involved_pins_[0].node_id_, op.Id());
    EXPECT_EQ(kResult.errors_[0].involved_pins_[0].pin_id_,
              op.InputPins()[1].id_);
    EXPECT_TRUE(kResult.errors_[0].involved_pins_[0].is_input_);
    EXPECT_NE(kResult.errors_[0].message_.find("right"), std::string::npos);
}

TEST_F(CValidationCapabilityTest, DetectsIncompatiblePinTypes) {
    core::Graph graph;

    // Connect Literal to int Variable, then change Literal property type to
    // string without updating the connected pin type
    core::Node &str_lit = SpawnCNode(graph, 2);
    core::Node &int_var = SpawnCNode(graph, 1);
    graph.Connect(str_lit.Id(), str_lit.OutputPins()[0].id_, int_var.Id(),
                  int_var.InputPins()[0].id_);
    str_lit.SetProperty(c_module::properties::kType,
                        {c_module::properties::kType, std::string("string")});
    str_lit.SetProperty(c_module::properties::kValue,
                        {c_module::properties::kValue, std::string("hello")});

    const auto kResult = validator_->Validate(graph);
    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasTypeError());
    ASSERT_EQ(kResult.ErrorCount(), 1U);
    EXPECT_NE(kResult.errors_[0].message_.find("string"), std::string::npos);
    EXPECT_NE(kResult.errors_[0].message_.find("int"), std::string::npos);
    EXPECT_EQ(kResult.errors_[0].involved_node_ids_.size(), 2U);
    EXPECT_EQ(kResult.errors_[0].involved_pins_.size(), 2U);
}

TEST_F(CValidationCapabilityTest,
       DetectsUnsupportedNodesAndIncompatibleValues) {
    core::Graph graph;

    // 1. Unsupported node type 99
    core::Node &unsupported = graph.CreateNode(99);

    // 2. Variable with reserved C++ keyword as identifier
    core::Node &bad_var = SpawnCNode(graph, 1);
    bad_var.SetProperty(c_module::properties::kName,
                        {c_module::properties::kName, std::string("class")});

    // 3. Operator with division by zero literal
    core::Node &lhs = SpawnCNode(graph, 2);
    lhs.SetProperty(c_module::properties::kValue,
                    {c_module::properties::kValue, 42.0});
    core::Node &zero = SpawnCNode(graph, 2);
    zero.SetProperty(c_module::properties::kValue,
                     {c_module::properties::kValue, 0.0});
    core::Node &div_op = SpawnCNode(graph, 3);
    div_op.SetProperty(c_module::properties::kOperator,
                       {c_module::properties::kOperator, std::string("/")});
    graph.Connect(lhs.Id(), lhs.OutputPins()[0].id_, div_op.Id(),
                  div_op.InputPins()[0].id_);
    graph.Connect(zero.Id(), zero.OutputPins()[0].id_, div_op.Id(),
                  div_op.InputPins()[1].id_);

    const auto kResult = validator_->Validate(graph);
    EXPECT_FALSE(kResult.is_valid_);
    EXPECT_TRUE(kResult.HasUnsupportedNodeError());
    EXPECT_TRUE(kResult.HasValueError());

    bool found_unsupported = false;
    bool found_keyword = false;
    bool found_div_zero = false;
    for (const auto &err : kResult.errors_) {
        if (err.type_ ==
            core::capa::ValidationError::ErrorType::kUnsupportedNode) {
            found_unsupported = (err.involved_node_ids_[0] == unsupported.Id());
        }
        if (err.message_.find("reserved keyword") != std::string::npos) {
            found_keyword = true;
        }
        if (err.message_.find("zero") != std::string::npos) {
            found_div_zero = true;
        }
    }
    EXPECT_TRUE(found_unsupported);
    EXPECT_TRUE(found_keyword);
    EXPECT_TRUE(found_div_zero);
}

TEST_F(CValidationCapabilityTest,
       ValidForLoopWithIndexFeedingConditionAndStepPasses) {
    core::Graph graph;

    core::Node &init_lit = SpawnCNode(graph, 2);
    init_lit.SetProperty(c_module::properties::kValue,
                         {c_module::properties::kValue, 0.0});

    core::Node &limit_lit = SpawnCNode(graph, 2);
    limit_lit.SetProperty(c_module::properties::kValue,
                          {c_module::properties::kValue, 10.0});

    core::Node &step_lit = SpawnCNode(graph, 2);
    step_lit.SetProperty(c_module::properties::kValue,
                         {c_module::properties::kValue, 1.0});

    // For node: inputs = [in(0), init(1), cond(2), step(3)]
    //           outputs = [body(0), index(1), exit(2)]
    core::Node &for_node = SpawnCNode(graph, 10);

    // Condition operator: index < 10 (returns bool)
    core::Node &cond_op = SpawnCNode(graph, 3);
    cond_op.SetProperty(c_module::properties::kOperator,
                        {c_module::properties::kOperator, std::string("<")});
    graph.SetPinType(cond_op.Id(), cond_op.OutputPins()[0].id_, false,
                     types_capa_->TypeId("bool"));

    // Step operator: index + 1 (returns int)
    core::Node &step_op = SpawnCNode(graph, 3);
    step_op.SetProperty(c_module::properties::kOperator,
                        {c_module::properties::kOperator, std::string("+")});

    // Connect init = 0
    graph.Connect(init_lit.Id(), init_lit.OutputPins()[0].id_, for_node.Id(),
                  for_node.InputPins()[1].id_);

    // Connect For.index -> cond_op.left, limit_lit -> cond_op.right, cond_op ->
    // For.cond
    graph.Connect(for_node.Id(), for_node.OutputPins()[1].id_, cond_op.Id(),
                  cond_op.InputPins()[0].id_);
    graph.Connect(limit_lit.Id(), limit_lit.OutputPins()[0].id_, cond_op.Id(),
                  cond_op.InputPins()[1].id_);
    graph.Connect(cond_op.Id(), cond_op.OutputPins()[0].id_, for_node.Id(),
                  for_node.InputPins()[2].id_);

    // Connect For.index -> step_op.left, step_lit -> step_op.right, step_op ->
    // For.step
    graph.Connect(for_node.Id(), for_node.OutputPins()[1].id_, step_op.Id(),
                  step_op.InputPins()[0].id_);
    graph.Connect(step_lit.Id(), step_lit.OutputPins()[0].id_, step_op.Id(),
                  step_op.InputPins()[1].id_);
    graph.Connect(step_op.Id(), step_op.OutputPins()[0].id_, for_node.Id(),
                  for_node.InputPins()[3].id_);

    const auto kResult = validator_->Validate(graph);
    EXPECT_TRUE(kResult.is_valid_);
    EXPECT_EQ(kResult.ErrorCount(), 0U);
}

}  // namespace
