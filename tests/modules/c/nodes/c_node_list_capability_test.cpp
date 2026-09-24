#include <gtest/gtest.h>
#include "nodes/c_node_list_capability.hpp"
#include "modules/capabilities/type_list_capability.hpp"

using namespace c_module;

class CNodeListCapabilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        types_capa_ = new core::capa::TypeListCapability();
        // C types
        types_capa_->RegisterType("void");
        types_capa_->RegisterType("int");
        types_capa_->RegisterType("float");
        types_capa_->RegisterType("bool");
        types_capa_->RegisterType("char");
        types_capa_->RegisterType("string");
        
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

class CNodeListCatalogueTest : public ::testing::TestWithParam<std::pair<int, std::string>> {
protected:
    void SetUp() override {
        types_capa_ = new core::capa::TypeListCapability();
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
