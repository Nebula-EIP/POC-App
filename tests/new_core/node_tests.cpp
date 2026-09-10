/**
 * @file node_tests.cpp
 * @brief Comprehensive tests for the Node class
 *
 * @author Created by JeanBizeul
 * @date Created on 11-09-2026
 */

#include <gtest/gtest.h>

#include "graph/graph.hpp"

// Helper fixture for Node tests that need a Graph
class NodeTest : public ::testing::Test {
   protected:
    core::Graph graph;
    core::Node &node = graph.CreateNode(42);
};

// ============================================================================
// Initialization & Properties Tests
// ============================================================================

TEST(NodeInitialization, NodeInitializesWithCorrectType)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(42);
    
    EXPECT_EQ(node.type(), 42);
}

TEST(NodeInitialization, NodeInitializesWithCorrectID)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    
    EXPECT_GE(node.id(), 1);
    EXPECT_LE(node.id(), 1000);  // Reasonable upper bound
}

TEST(NodeInitialization, NodeInitializesWithNoPins)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    
    EXPECT_EQ(node.inputPinsCount(), 0);
    EXPECT_EQ(node.outputPinsCount(), 0);
}

TEST(NodeInitialization, NodeInitializesWithNoProperties)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    
    // No property should exist
    EXPECT_FALSE(node.hasProperty(1));
    EXPECT_EQ(node.property(1), nullptr);
}

// ============================================================================
// Pin Existence Tests
// ============================================================================

TEST_F(NodeTest, InputPinExistsReturnsTrueForExisting)
{
    core::PinId pinId = graph.AddInputPin(node.id(), "test", 0);
    EXPECT_TRUE(node.inputPinExists(pinId));
}

TEST_F(NodeTest, InputPinExistsReturnsFalseForNonExistent)
{
    EXPECT_FALSE(node.inputPinExists(999));
}

TEST_F(NodeTest, OutputPinExistsReturnsTrueForExisting)
{
    core::PinId pinId = graph.AddOutputPin(node.id(), "test", 0);
    EXPECT_TRUE(node.outputPinExists(pinId));
}

TEST_F(NodeTest, OutputPinExistsReturnsFalseForNonExistent)
{
    EXPECT_FALSE(node.outputPinExists(999));
}

// ============================================================================
// Pin Retrieval Tests
// ============================================================================

TEST_F(NodeTest, InputPinReturnsConstPointerForExisting)
{
    core::PinId pinId = graph.AddInputPin(node.id(), "test", 42);
    const core::Pin *pin = node.inputPin(pinId);
    
    EXPECT_NE(pin, nullptr);
    EXPECT_EQ(pin->id, pinId);
}

TEST_F(NodeTest, InputPinReturnsNullptrForNonExistent)
{
    const core::Pin *pin = node.inputPin(999);
    EXPECT_EQ(pin, nullptr);
}

TEST_F(NodeTest, OutputPinReturnsConstPointerForExisting)
{
    core::PinId pinId = graph.AddOutputPin(node.id(), "test", 42);
    const core::Pin *pin = node.outputPin(pinId);
    
    EXPECT_NE(pin, nullptr);
    EXPECT_EQ(pin->id, pinId);
}

TEST_F(NodeTest, OutputPinReturnsNullptrForNonExistent)
{
    const core::Pin *pin = node.outputPin(999);
    EXPECT_EQ(pin, nullptr);
}

TEST_F(NodeTest, InputPinsReturnsVectorOfAllInputPins)
{
    graph.AddInputPin(node.id(), "pin1", 0);
    graph.AddInputPin(node.id(), "pin2", 0);
    graph.AddInputPin(node.id(), "pin3", 0);
    
    const std::vector<core::Pin> &pins = node.inputPins();
    EXPECT_EQ(pins.size(), 3);
}

TEST_F(NodeTest, OutputPinsReturnsVectorOfAllOutputPins)
{
    graph.AddOutputPin(node.id(), "pin1", 0);
    graph.AddOutputPin(node.id(), "pin2", 0);
    
    const std::vector<core::Pin> &pins = node.outputPins();
    EXPECT_EQ(pins.size(), 2);
}

TEST_F(NodeTest, InputPinsCountReturnsCorrectCount)
{
    EXPECT_EQ(node.inputPinsCount(), 0);
    
    graph.AddInputPin(node.id(), "pin1", 0);
    EXPECT_EQ(node.inputPinsCount(), 1);
    
    graph.AddInputPin(node.id(), "pin2", 0);
    EXPECT_EQ(node.inputPinsCount(), 2);
}

TEST_F(NodeTest, OutputPinsCountReturnsCorrectCount)
{
    EXPECT_EQ(node.outputPinsCount(), 0);
    
    graph.AddOutputPin(node.id(), "pin1", 0);
    EXPECT_EQ(node.outputPinsCount(), 1);
    
    graph.AddOutputPin(node.id(), "pin2", 0);
    EXPECT_EQ(node.outputPinsCount(), 2);
}

// ============================================================================
// Property Management Tests
// ============================================================================

TEST_F(NodeTest, AddPropertyAddsPropertyAndReturnsUniqueID)
{
    core::Property prop;
    prop.type_id = 1;
    
    core::PropertyId propId = node.AddProperty(prop);
    EXPECT_GT(propId, 0);
    EXPECT_TRUE(node.hasProperty(propId));
}

TEST_F(NodeTest, AddPropertyIncrementsPropertyCounterCorrectly)
{
    core::Property prop;
    prop.type_id = 1;
    
    core::PropertyId prop1 = node.AddProperty(prop);
    core::PropertyId prop2 = node.AddProperty(prop);
    core::PropertyId prop3 = node.AddProperty(prop);
    
    EXPECT_LT(prop1, prop2);
    EXPECT_LT(prop2, prop3);
}

TEST_F(NodeTest, AddPropertyMultipleAssignsUniqueIDs)
{
    core::Property prop;
    prop.type_id = 1;
    
    core::PropertyId id1 = node.AddProperty(prop);
    core::PropertyId id2 = node.AddProperty(prop);
    core::PropertyId id3 = node.AddProperty(prop);
    
    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
    EXPECT_NE(id1, id3);
}

TEST_F(NodeTest, SetPropertyUpdatesExistingProperty)
{
    core::Property prop1;
    prop1.type_id = 1;
    core::PropertyId propId = node.AddProperty(prop1);
    
    core::Property prop2;
    prop2.type_id = 2;
    node.SetProperty(propId, prop2);
    
    const core::Property *retrieved = node.property(propId);
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->type_id, 2);
}

TEST_F(NodeTest, SetPropertyCreatesPropertyIfNotExists)
{
    core::Property prop;
    prop.type_id = 42;
    
    node.SetProperty(999, prop);
    
    const core::Property *retrieved = node.property(999);
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->type_id, 42);
}

TEST_F(NodeTest, RemovePropertyRemovesProperty)
{
    core::Property prop;
    prop.type_id = 1;
    core::PropertyId propId = node.AddProperty(prop);
    
    EXPECT_TRUE(node.hasProperty(propId));
    node.RemoveProperty(propId);
    EXPECT_FALSE(node.hasProperty(propId));
}

TEST_F(NodeTest, RemovePropertyDoesNothingForNonExistent)
{
    // Should not throw
    node.RemoveProperty(999);
    EXPECT_FALSE(node.hasProperty(999));
}

TEST_F(NodeTest, HasPropertyReturnsTrueForExisting)
{
    core::Property prop;
    prop.type_id = 1;
    core::PropertyId propId = node.AddProperty(prop);
    
    EXPECT_TRUE(node.hasProperty(propId));
}

TEST_F(NodeTest, HasPropertyReturnsFalseForNonExistent)
{
    EXPECT_FALSE(node.hasProperty(999));
}

TEST_F(NodeTest, PropertyReturnsPointerForExisting)
{
    core::Property prop;
    prop.type_id = 42;
    core::PropertyId propId = node.AddProperty(prop);
    
    core::Property *retrieved = node.property(propId);
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->type_id, 42);
}

TEST_F(NodeTest, PropertyReturnsNullptrForNonExistent)
{
    core::Property *retrieved = node.property(999);
    EXPECT_EQ(retrieved, nullptr);
}

TEST_F(NodeTest, PropertyConstReturnsConstPointerForExisting)
{
    core::Property prop;
    prop.type_id = 42;
    core::PropertyId propId = node.AddProperty(prop);
    
    const core::Node &constNode = node;
    const core::Property *retrieved = constNode.property(propId);
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->type_id, 42);
}

// ============================================================================
// Pin Internal Management Tests
// ============================================================================

TEST_F(NodeTest, AddInputPinAssignsCorrectID)
{
    core::PinId pin1 = graph.AddInputPin(node.id(), "pin1", 0);
    core::PinId pin2 = graph.AddInputPin(node.id(), "pin2", 0);
    
    EXPECT_NE(pin1, pin2);
    EXPECT_GT(pin1, 0);
    EXPECT_GT(pin2, 0);
}

TEST_F(NodeTest, AddInputPinAddsToInternalVector)
{
    EXPECT_EQ(node.inputPinsCount(), 0);
    
    graph.AddInputPin(node.id(), "pin", 0);
    EXPECT_EQ(node.inputPinsCount(), 1);
    
    graph.AddInputPin(node.id(), "pin2", 0);
    EXPECT_EQ(node.inputPinsCount(), 2);
}

TEST_F(NodeTest, AddOutputPinAssignsCorrectID)
{
    core::PinId pin1 = graph.AddOutputPin(node.id(), "pin1", 0);
    core::PinId pin2 = graph.AddOutputPin(node.id(), "pin2", 0);
    
    EXPECT_NE(pin1, pin2);
    EXPECT_GT(pin1, 0);
    EXPECT_GT(pin2, 0);
}

TEST_F(NodeTest, AddOutputPinAddsToInternalVector)
{
    EXPECT_EQ(node.outputPinsCount(), 0);
    
    graph.AddOutputPin(node.id(), "pin", 0);
    EXPECT_EQ(node.outputPinsCount(), 1);
    
    graph.AddOutputPin(node.id(), "pin2", 0);
    EXPECT_EQ(node.outputPinsCount(), 2);
}

TEST_F(NodeTest, RemoveInputPinRemovesFromInternalVector)
{
    core::PinId pinId = graph.AddInputPin(node.id(), "pin", 0);
    EXPECT_EQ(node.inputPinsCount(), 1);
    
    graph.RemoveInputPin(node.id(), pinId);
    EXPECT_EQ(node.inputPinsCount(), 0);
}

TEST_F(NodeTest, RemoveOutputPinRemovesFromInternalVector)
{
    core::PinId pinId = graph.AddOutputPin(node.id(), "pin", 0);
    EXPECT_EQ(node.outputPinsCount(), 1);
    
    graph.RemoveOutputPin(node.id(), pinId);
    EXPECT_EQ(node.outputPinsCount(), 0);
}

// ============================================================================
// Getter Tests
// ============================================================================

TEST_F(NodeTest, IdReturnsCorrectNodeID)
{
    core::NodeId nodeId = node.id();
    EXPECT_EQ(node.id(), nodeId);
}

TEST_F(NodeTest, TypeReturnsCorrectNodeType)
{
    EXPECT_EQ(node.type(), 42);
}

// ============================================================================
// Complex Scenarios
// ============================================================================

TEST_F(NodeTest, MixPropertyAndPinOperations)
{
    // Add pins
    core::PinId inPin = graph.AddInputPin(node.id(), "in", 0);
    core::PinId outPin = graph.AddOutputPin(node.id(), "out", 0);
    
    // Add properties
    core::Property prop1;
    prop1.type_id = 1;
    core::PropertyId propId1 = node.AddProperty(prop1);
    
    core::Property prop2;
    prop2.type_id = 2;
    core::PropertyId propId2 = node.AddProperty(prop2);
    
    // Verify all exist
    EXPECT_TRUE(node.inputPinExists(inPin));
    EXPECT_TRUE(node.outputPinExists(outPin));
    EXPECT_TRUE(node.hasProperty(propId1));
    EXPECT_TRUE(node.hasProperty(propId2));
    
    // Remove one of each
    graph.RemoveInputPin(node.id(), inPin);
    node.RemoveProperty(propId1);
    
    // Verify correct ones remain
    EXPECT_FALSE(node.inputPinExists(inPin));
    EXPECT_TRUE(node.outputPinExists(outPin));
    EXPECT_FALSE(node.hasProperty(propId1));
    EXPECT_TRUE(node.hasProperty(propId2));
}

TEST_F(NodeTest, MultipleInputAndOutputPins)
{
    const int NUM_PINS = 5;
    
    std::vector<core::PinId> inPins, outPins;
    
    for (int i = 0; i < NUM_PINS; ++i) {
        inPins.push_back(graph.AddInputPin(node.id(), "in" + std::to_string(i), 0));
        outPins.push_back(graph.AddOutputPin(node.id(), "out" + std::to_string(i), 0));
    }
    
    EXPECT_EQ(node.inputPinsCount(), NUM_PINS);
    EXPECT_EQ(node.outputPinsCount(), NUM_PINS);
    
    // Verify all pins exist
    for (int i = 0; i < NUM_PINS; ++i) {
        EXPECT_TRUE(node.inputPinExists(inPins[i]));
        EXPECT_TRUE(node.outputPinExists(outPins[i]));
    }
}

TEST_F(NodeTest, PropertyLifecycle)
{
    const int NUM_PROPS = 3;
    std::vector<core::PropertyId> propIds;
    
    // Add properties
    for (int i = 0; i < NUM_PROPS; ++i) {
        core::Property prop;
        prop.type_id = i;
        propIds.push_back(node.AddProperty(prop));
    }
    
    // Verify all added
    EXPECT_EQ(propIds.size(), NUM_PROPS);
    for (const auto &id : propIds) {
        EXPECT_TRUE(node.hasProperty(id));
    }
    
    // Remove middle property
    node.RemoveProperty(propIds[1]);
    EXPECT_TRUE(node.hasProperty(propIds[0]));
    EXPECT_FALSE(node.hasProperty(propIds[1]));
    EXPECT_TRUE(node.hasProperty(propIds[2]));
    
    // Add new property
    core::Property newProp;
    newProp.type_id = 99;
    core::PropertyId newId = node.AddProperty(newProp);
    EXPECT_TRUE(node.hasProperty(newId));
}
