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
    
    capability.RegisterNode(1, "TestNode", "A test node", config);
    
    auto nodes = capability.GetAvailableNodes();
    ASSERT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes[0].type_, 1);
    EXPECT_EQ(nodes[0].name_, "TestNode");
    EXPECT_EQ(nodes[0].description_, "A test node");
    
    auto retrieved_config = capability.GetNodeConfiguration(1);
    ASSERT_EQ(retrieved_config.input_pins_.size(), 1);
    EXPECT_EQ(retrieved_config.input_pins_[0].id, 1);
    EXPECT_EQ(retrieved_config.input_pins_[0].name, "in");
    EXPECT_EQ(retrieved_config.input_pins_[0].type, 10);
}

TEST(NodeListCapabilityTest, DuplicateNodeRegistrationThrows) {
    NodeListCapability capability;
    NodeConfiguration config;
    
    capability.RegisterNode(1, "TestNode", "A test node", config);
    
    EXPECT_THROW(capability.RegisterNode(1, "TestNode2", "Another test node", config), NodeAlreadyExistsException);
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
    // There are no getters for property types in INodeListCapability,
    // so we just verify it doesn't crash.
}
