/**
 * @file c_node_list_capability_test.cpp
 * @brief Unit tests for the C module node list capability.
 *
 * @author Created by mathys-f
 * @date Created on 21-09-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 29-10-2026
 */

#include "nodes/c_node_list_capability.hpp"

#include <gtest/gtest.h>

#include <any>
#include <string>

#include "modules/capabilities/type_list_capability.hpp"

using namespace c_module;

namespace {

/// Types the C nodes rely on.
const std::vector<std::string_view> kCTypes = {
    "void", "int", "float", "bool", "char", "string",
};

/// Build the type capability and assign an ID to every type, as the core does.
core::capa::TypeListCapability *CreateRegisteredTypes() {
    auto *types = new core::capa::TypeListCapability(kCTypes);
    for (std::size_t i = 0; i < kCTypes.size(); ++i) {
        types->RegisterType(static_cast<core::DataType>(i + 1));
    }
    return types;
}

}  // namespace

class CNodeListCapabilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        types_capa_ = CreateRegisteredTypes();
        
        node_capa_ = new CNodeListCapability(types_capa_);
        
        // Register the 10 nodes to assign them IDs
        for (int i = 0; i < 10; ++i) {
            node_capa_->RegisterNode(i + 1);
        }
    }
    
    void TearDown() override {
        delete node_capa_;
        delete types_capa_;
    }

    core::capa::TypeListCapability *types_capa_;
    CNodeListCapability *node_capa_;
};

TEST_F(CNodeListCapabilityTest, CheckCatalogueSizeAndNames) {
    auto nodes = node_capa_->GetAvailableNodes();
    ASSERT_EQ(nodes.size(), 10);
    
    std::vector<std::string> expected_names = {
        "Variable", "Literal", "Operator", "Condition", "Loop",
        "Function", "FunctionInput", "FunctionOutput", "Print", "For"
    };
    
    for (size_t i = 0; i < expected_names.size(); ++i) {
        EXPECT_EQ(nodes[i].name_, expected_names[i]);
    }
}

TEST_F(CNodeListCapabilityTest, VariableConfig) {
    auto config = node_capa_->GetNodeConfiguration(1); // Variable is 1
    ASSERT_EQ(config.input_pins_.size(), 1);
    EXPECT_EQ(config.input_pins_[0].name_, "in");
    EXPECT_EQ(config.input_pins_[0].type_, types_capa_->TypeId("int"));
    ASSERT_EQ(config.output_pins_.size(), 1);
    EXPECT_EQ(config.output_pins_[0].name_, "out");
    EXPECT_EQ(config.output_pins_[0].type_, types_capa_->TypeId("int"));
    EXPECT_TRUE(config.default_properties_.count(properties::kName));
    EXPECT_TRUE(config.default_properties_.count(properties::kType));
}

TEST_F(CNodeListCapabilityTest, OperatorConfig) {
    auto config = node_capa_->GetNodeConfiguration(3); // Operator is 3
    EXPECT_EQ(config.input_pins_.size(), 2);
    EXPECT_EQ(config.output_pins_.size(), 1);
    EXPECT_TRUE(config.default_properties_.count(properties::kOperator));
    EXPECT_TRUE(config.default_properties_.count(properties::kType));
}

TEST_F(CNodeListCapabilityTest, ControlFlowConfigsHaveFlowPins) {
    auto condition = node_capa_->GetNodeConfiguration(4); // Condition is 4
    ASSERT_EQ(condition.input_pins_.size(), 2);
    EXPECT_EQ(condition.input_pins_[0].name_, "in");
    EXPECT_EQ(condition.input_pins_[0].type_, types_capa_->TypeId("void"));
    EXPECT_EQ(condition.input_pins_[1].name_, "cond");
    EXPECT_EQ(condition.input_pins_[1].type_, types_capa_->TypeId("bool"));
    ASSERT_EQ(condition.output_pins_.size(), 3);
    EXPECT_EQ(condition.output_pins_[0].name_, "true");
    EXPECT_EQ(condition.output_pins_[1].name_, "false");
    EXPECT_EQ(condition.output_pins_[2].name_, "out");

    auto loop = node_capa_->GetNodeConfiguration(5); // Loop is 5
    ASSERT_EQ(loop.input_pins_.size(), 2);
    EXPECT_EQ(loop.input_pins_[0].name_, "in");
    EXPECT_EQ(loop.input_pins_[0].type_, types_capa_->TypeId("void"));
    EXPECT_EQ(loop.input_pins_[1].name_, "cond");
    EXPECT_EQ(loop.input_pins_[1].type_, types_capa_->TypeId("bool"));
    ASSERT_EQ(loop.output_pins_.size(), 2);
    EXPECT_EQ(loop.output_pins_[0].name_, "body");
    EXPECT_EQ(loop.output_pins_[1].name_, "exit");
}

TEST_F(CNodeListCapabilityTest, ForConfigExposesFlowAndIteratorPins) {
    auto config = node_capa_->GetNodeConfiguration(10); // For is 10
    ASSERT_EQ(config.input_pins_.size(), 4);
    EXPECT_EQ(config.input_pins_[0].name_, "in");
    EXPECT_EQ(config.input_pins_[0].type_, types_capa_->TypeId("void"));
    EXPECT_EQ(config.input_pins_[1].name_, "init");
    EXPECT_EQ(config.input_pins_[1].type_, types_capa_->TypeId("int"));
    EXPECT_EQ(config.input_pins_[2].name_, "cond");
    EXPECT_EQ(config.input_pins_[2].type_, types_capa_->TypeId("bool"));
    EXPECT_EQ(config.input_pins_[3].name_, "step");
    EXPECT_EQ(config.input_pins_[3].type_, types_capa_->TypeId("int"));

    ASSERT_EQ(config.output_pins_.size(), 3);
    EXPECT_EQ(config.output_pins_[0].name_, "body");
    EXPECT_EQ(config.output_pins_[0].type_, types_capa_->TypeId("void"));
    EXPECT_EQ(config.output_pins_[1].name_, "index");
    EXPECT_EQ(config.output_pins_[1].type_, types_capa_->TypeId("int"));
    EXPECT_EQ(config.output_pins_[2].name_, "exit");
    EXPECT_EQ(config.output_pins_[2].type_, types_capa_->TypeId("void"));
    ASSERT_TRUE(config.default_properties_.count(properties::kName));
    EXPECT_EQ(std::any_cast<std::string>(
                  config.default_properties_.at(properties::kName).value_),
              "i");
}

TEST_F(CNodeListCapabilityTest, TypedNodesStartAsInt) {
    for (const core::NodeType type : {1, 2}) {  // Variable, Literal
        auto config = node_capa_->GetNodeConfiguration(type);
        ASSERT_TRUE(config.default_properties_.count(properties::kType));
        EXPECT_EQ(std::any_cast<std::string>(
                      config.default_properties_.at(properties::kType).value_),
                  "int");
        EXPECT_EQ(config.output_pins_[0].type_, types_capa_->TypeId("int"));
    }
    auto variable = node_capa_->GetNodeConfiguration(1);
    EXPECT_EQ(std::any_cast<std::string>(
                  variable.default_properties_.at(properties::kName).value_),
              "variable");
}

TEST_F(CNodeListCapabilityTest, TypedNodesOutputTheirStoredType) {
    core::PropertyMap properties{
        {properties::kType, {properties::kType, std::string("float")}}};

    auto literal = node_capa_->ResolveNodeConfiguration(2, properties);
    EXPECT_EQ(literal.output_pins_[0].type_, types_capa_->TypeId("float"));
    auto variable = node_capa_->ResolveNodeConfiguration(1, properties);
    EXPECT_EQ(variable.input_pins_[0].type_, types_capa_->TypeId("float"));
    EXPECT_EQ(variable.output_pins_[0].type_, types_capa_->TypeId("float"));

    // Unknown or void types fall back to int.
    properties[properties::kType] = {properties::kType, std::string("void")};
    literal = node_capa_->ResolveNodeConfiguration(2, properties);
    EXPECT_EQ(literal.output_pins_[0].type_, types_capa_->TypeId("int"));
    literal = node_capa_->ResolveNodeConfiguration(2, {});
    EXPECT_EQ(literal.output_pins_[0].type_, types_capa_->TypeId("int"));
}

TEST_F(CNodeListCapabilityTest, OperatorResolvesArithmeticComparisonAndLogicalTypes) {
    core::PropertyMap arithmetic{
        {properties::kOperator, {properties::kOperator, std::string("+")}},
        {properties::kType, {properties::kType, std::string("float")}}};
    auto op = node_capa_->ResolveNodeConfiguration(3, arithmetic);
    EXPECT_EQ(op.input_pins_[0].type_, types_capa_->TypeId("float"));
    EXPECT_EQ(op.input_pins_[1].type_, types_capa_->TypeId("float"));
    EXPECT_EQ(op.output_pins_[0].type_, types_capa_->TypeId("float"));

    core::PropertyMap comparison{
        {properties::kOperator, {properties::kOperator, std::string("<")}},
        {properties::kType, {properties::kType, std::string("int")}}};
    op = node_capa_->ResolveNodeConfiguration(3, comparison);
    EXPECT_EQ(op.input_pins_[0].type_, types_capa_->TypeId("int"));
    EXPECT_EQ(op.input_pins_[1].type_, types_capa_->TypeId("int"));
    EXPECT_EQ(op.output_pins_[0].type_, types_capa_->TypeId("bool"));

    core::PropertyMap logical{
        {properties::kOperator, {properties::kOperator, std::string("&&")}},
        {properties::kType, {properties::kType, std::string("int")}}};
    op = node_capa_->ResolveNodeConfiguration(3, logical);
    EXPECT_EQ(op.input_pins_[0].type_, types_capa_->TypeId("bool"));
    EXPECT_EQ(op.input_pins_[1].type_, types_capa_->TypeId("bool"));
    EXPECT_EQ(op.output_pins_[0].type_, types_capa_->TypeId("bool"));
}

TEST_F(CNodeListCapabilityTest, FunctionIOAndPrintResolveDataPinTypes) {
    core::PropertyMap float_prop{
        {properties::kType, {properties::kType, std::string("float")}}};

    auto func_in = node_capa_->ResolveNodeConfiguration(7, float_prop);
    ASSERT_EQ(func_in.output_pins_.size(), 2);
    EXPECT_EQ(func_in.output_pins_[0].type_, types_capa_->TypeId("void"));
    EXPECT_EQ(func_in.output_pins_[1].type_, types_capa_->TypeId("float"));

    auto func_out = node_capa_->ResolveNodeConfiguration(8, float_prop);
    ASSERT_EQ(func_out.input_pins_.size(), 2);
    EXPECT_EQ(func_out.input_pins_[0].type_, types_capa_->TypeId("void"));
    EXPECT_EQ(func_out.input_pins_[1].type_, types_capa_->TypeId("float"));

    auto print_cfg = node_capa_->ResolveNodeConfiguration(9, float_prop);
    ASSERT_EQ(print_cfg.input_pins_.size(), 2);
    EXPECT_EQ(print_cfg.input_pins_[0].type_, types_capa_->TypeId("void"));
    EXPECT_EQ(print_cfg.input_pins_[1].type_, types_capa_->TypeId("float"));
    ASSERT_EQ(print_cfg.output_pins_.size(), 1);
    EXPECT_EQ(print_cfg.output_pins_[0].type_, types_capa_->TypeId("void"));
}

class CNodeListCatalogueTest : public ::testing::TestWithParam<std::pair<int, std::string>> {
protected:
    void SetUp() override {
        types_capa_ = CreateRegisteredTypes();
        node_capa_ = new CNodeListCapability(types_capa_);
        for (int i = 0; i < 10; ++i) {
            node_capa_->RegisterNode(i + 1);
        }
    }
    
    void TearDown() override {
        delete node_capa_;
        delete types_capa_;
    }

    core::capa::TypeListCapability *types_capa_;
    CNodeListCapability *node_capa_;
};

TEST_P(CNodeListCatalogueTest, HasNodeInCatalogue) {
    auto expected_name = GetParam().second;
    auto nodes = node_capa_->GetAvailableNodes();
    
    bool found = false;
    for (const auto& node : nodes) {
        if (node.name_ == expected_name) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Node " << expected_name << " not found in catalogue.";
}

INSTANTIATE_TEST_SUITE_P(
    CatalogueNodes,
    CNodeListCatalogueTest,
    ::testing::Values(
        std::make_pair(1, "Variable"),
        std::make_pair(2, "Literal"),
        std::make_pair(3, "Operator"),
        std::make_pair(4, "Condition"),
        std::make_pair(5, "Loop"),
        std::make_pair(6, "Function"),
        std::make_pair(7, "FunctionInput"),
        std::make_pair(8, "FunctionOutput"),
        std::make_pair(9, "Print"),
        std::make_pair(10, "For")
    )
);
