#include <gtest/gtest.h>
#include "modules/capabilities/node_list_capability.hpp"
#include "exception/graph_exception/node_exception.hpp"

using namespace core;
using namespace core::capa;

TEST(NodeListCapabilityTest, RegisterAndRetrieveNode) {
    NodeListCapability capability;
    
    NodeConfiguration config;
    config.input_pins_.push_back({1, "in", 10});
    config.output_pins_.push_back({2, "out", 10});
    
    capability.RegisterNode("TestNode", "A test node", config);
    
    auto name_ptr1 = capability.RegisterNode(42);
    ASSERT_NE(name_ptr1, nullptr);
    EXPECT_EQ(*name_ptr1, "TestNode");
    
    auto name_ptr2 = capability.RegisterNode(43);
    EXPECT_EQ(name_ptr2, nullptr);
    
    auto nodes = capability.GetAvailableNodes();
    ASSERT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes[0].type_, 42);
    EXPECT_EQ(nodes[0].name_, "TestNode");
    EXPECT_EQ(nodes[0].description_, "A test node");
    
    auto retrieved_config = capability.GetNodeConfiguration(42);
    ASSERT_EQ(retrieved_config.input_pins_.size(), 1);
    EXPECT_EQ(retrieved_config.input_pins_[0].id_, 1);
    EXPECT_EQ(retrieved_config.input_pins_[0].name_, "in");
    EXPECT_EQ(retrieved_config.input_pins_[0].type_, 10);
}

TEST(NodeListCapabilityTest, DuplicateNodeRegistrationThrows) {
    NodeListCapability capability;
    NodeConfiguration config;
    
    capability.RegisterNode("TestNode", "A test node", config);
    
    EXPECT_THROW(capability.RegisterNode("TestNode", "Another test node", config), NodeAlreadyExistsException);
}

TEST(NodeListCapabilityTest, UnknownNodeConfigurationThrows) {
    NodeListCapability capability;
    
    EXPECT_THROW(capability.GetNodeConfiguration(999), NodeTypeException);
}

TEST(NodeListCapabilityTest, InitializePropertyTypes) {
    NodeListCapability capability;
    
    std::unordered_map<std::string, PropertyTypeId> property_types = {
        {"int", 1},
        {"float", 2}
    };
    
    capability.InitializePropertyTypes(property_types);
}
