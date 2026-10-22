#include <gtest/gtest.h>
#include <any>
#include <string>
#include "nodes/c_node_list_capability.hpp"
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
    EXPECT_EQ(config.output_pins_.size(), 1);
    EXPECT_TRUE(config.default_properties_.count(properties::kName));
}

TEST_F(CNodeListCapabilityTest, OperatorConfig) {
    auto config = node_capa_->GetNodeConfiguration(3); // Operator is 3
    EXPECT_EQ(config.input_pins_.size(), 2);
    EXPECT_EQ(config.output_pins_.size(), 1);
    EXPECT_TRUE(config.default_properties_.count(properties::kOperator));
}

TEST_F(CNodeListCapabilityTest, ForConfig) {
    auto config = node_capa_->GetNodeConfiguration(10); // For is 10
    EXPECT_EQ(config.input_pins_.size(), 3);
    EXPECT_EQ(config.input_pins_[0].name_, "init");
    EXPECT_EQ(config.input_pins_[1].name_, "cond");
    EXPECT_EQ(config.input_pins_[2].name_, "step");
    EXPECT_EQ(config.output_pins_.size(), 1);
    EXPECT_EQ(config.output_pins_[0].name_, "body");
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
    EXPECT_EQ(variable.output_pins_[0].type_, types_capa_->TypeId("float"));

    // Unknown or void types fall back to int.
    properties[properties::kType] = {properties::kType, std::string("void")};
    literal = node_capa_->ResolveNodeConfiguration(2, properties);
    EXPECT_EQ(literal.output_pins_[0].type_, types_capa_->TypeId("int"));
    literal = node_capa_->ResolveNodeConfiguration(2, {});
    EXPECT_EQ(literal.output_pins_[0].type_, types_capa_->TypeId("int"));
}

TEST_F(CNodeListCapabilityTest, OtherNodesIgnoreTheTypeProperty) {
    core::PropertyMap properties{
        {properties::kType, {properties::kType, std::string("float")}}};
    auto op = node_capa_->ResolveNodeConfiguration(3, properties);
    EXPECT_EQ(op.output_pins_[0].type_, types_capa_->TypeId("int"));
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
