/**
 * @file graph_tests.cpp
 * @brief Comprehensive tests for the Graph class
 *
 * @author Created by JeanBizeul
 * @date Created on 11-09-2026
 */

#include <gtest/gtest.h>

#include "graph/graph.hpp"
#include "exception/graph_exception/node_exception.hpp"
#include "exception/graph_exception/connection_exception.hpp"

// ============================================================================
// Initialization & Lifecycle Tests
// ============================================================================

TEST(GraphInitialization, GraphInitializesWithNoNodes)
{
    core::Graph graph;
    EXPECT_EQ(graph.GetAllNodes().size(), 0);
}

TEST(GraphInitialization, GraphInitializesWithCorrectCounters)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    
    EXPECT_EQ(node1.Id(), 1);
    EXPECT_EQ(node2.Id(), 2);
}

TEST(GraphInitialization, GraphDestructorWorksCorrectly)
{
    {
        core::Graph graph;
        graph.CreateNode(1);
        graph.CreateNode(2);
        graph.CreateNode(3);
    }
    // No crash means destructor works
    SUCCEED();
}

// ============================================================================
// Node Management Tests
// ============================================================================

TEST(GraphNodeManagement, CreateNodeCreatesNodeWithCorrectType)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(42);
    
    EXPECT_EQ(node.Type(), 42);
}

TEST(GraphNodeManagement, CreateNodeIncrementsNodeIDCorrectly)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    core::Node &node3 = graph.CreateNode(3);
    
    EXPECT_EQ(node1.Id(), 1);
    EXPECT_EQ(node2.Id(), 2);
    EXPECT_EQ(node3.Id(), 3);
}

TEST(GraphNodeManagement, CreateNodeReturnsReference)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    
    EXPECT_EQ(node.Type(), 1);
    EXPECT_GE(node.Id(), 1);
}

TEST(GraphNodeManagement, CreateMultipleNodesAssignsUniqueIds)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(1);
    core::Node &node3 = graph.CreateNode(1);
    
    EXPECT_NE(node1.Id(), node2.Id());
    EXPECT_NE(node2.Id(), node3.Id());
    EXPECT_NE(node1.Id(), node3.Id());
}

TEST(GraphNodeManagement, RemoveNodeRemovesExistingNode)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    core::NodeId nodeId = node.Id();
    
    EXPECT_TRUE(graph.HasNode(nodeId));
    bool removed = graph.RemoveNode(nodeId);
    
    EXPECT_TRUE(removed);
    EXPECT_FALSE(graph.HasNode(nodeId));
}

TEST(GraphNodeManagement, RemoveNodeReturnsFalseForNonExistent)
{
    core::Graph graph;
    bool removed = graph.RemoveNode(999);
    
    EXPECT_FALSE(removed);
}

TEST(GraphNodeManagement, RemoveNodeRemovesAllAssociatedConnections)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    core::NodeId id1 = node1.Id();
    core::NodeId id2 = node2.Id();
    
    graph.AddOutputPin(id1, "out", 0);
    graph.AddInputPin(id2, "in", 0);
    graph.Connect(id1, 1, id2, 1);
    
    EXPECT_EQ(graph.GetAllConnections().size(), 1);
    EXPECT_TRUE(graph.RemoveNode(id1));
    EXPECT_EQ(graph.GetAllConnections().size(), 0);
}

TEST(GraphNodeManagement, HasNodeReturnsTrueForExisting)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    
    EXPECT_TRUE(graph.HasNode(node.Id()));
}

TEST(GraphNodeManagement, HasNodeReturnsFalseForNonExistent)
{
    core::Graph graph;
    EXPECT_FALSE(graph.HasNode(999));
}

TEST(GraphNodeManagement, NodeReturnsPointerForExisting)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    
    core::Node *retrieved = graph.GetNode(node.Id());
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->Id(), node.Id());
}

TEST(GraphNodeManagement, NodeReturnsNullptrForNonExistent)
{
    core::Graph graph;
    core::Node *retrieved = graph.GetNode(999);
    
    EXPECT_EQ(retrieved, nullptr);
}

TEST(GraphNodeManagement, NodeConstReturnsConstPointer)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    const core::Graph &constGraph = graph;
    
    const core::Node *retrieved = constGraph.GetNode(node.Id());
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->Id(), node.Id());
}

TEST(GraphNodeManagement, NodesReturnsAllCreatedNodes)
{
    core::Graph graph;
    graph.CreateNode(1);
    graph.CreateNode(2);
    graph.CreateNode(3);
    
    EXPECT_EQ(graph.GetAllNodes().size(), 3);
}

// ============================================================================
// Pin Management Tests (Input Pins)
// ============================================================================

TEST(GraphInputPinManagement, AddInputPinAddsToExistingNode)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    
    core::PinId pinId = graph.AddInputPin(node.Id(), "test_pin", 0);
    EXPECT_GT(pinId, 0);
    EXPECT_TRUE(node.InputPinExists(pinId));
}

TEST(GraphInputPinManagement, AddInputPinReturnsValidPinID)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    
    core::PinId pinId = graph.AddInputPin(node.Id(), "pin1", 0);
    EXPECT_GT(pinId, 0);
}

TEST(GraphInputPinManagement, AddInputPinThrowsForInvalidNode)
{
    core::Graph graph;
    
    EXPECT_THROW(
        graph.AddInputPin(999, "pin", 0),
        core::NodeNotFoundException
    );
}

TEST(GraphInputPinManagement, RemoveInputPinRemovesExisting)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    core::PinId pinId = graph.AddInputPin(node.Id(), "pin", 0);
    
    EXPECT_TRUE(node.InputPinExists(pinId));
    graph.RemoveInputPin(node.Id(), pinId);
    EXPECT_FALSE(node.InputPinExists(pinId));
}

TEST(GraphInputPinManagement, RemoveInputPinThrowsForInvalidNode)
{
    core::Graph graph;
    
    EXPECT_THROW(
        graph.RemoveInputPin(999, 1),
        core::NodeNotFoundException
    );
}

TEST(GraphInputPinManagement, RemoveInputPinRemovesAssociatedConnections)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin = graph.AddInputPin(node2.Id(), "in", 0);
    
    graph.Connect(node1.Id(), outPin, node2.Id(), inPin);
    EXPECT_EQ(graph.GetAllConnections().size(), 1);
    
    graph.RemoveInputPin(node2.Id(), inPin);
    EXPECT_EQ(graph.GetAllConnections().size(), 0);
}

TEST(GraphInputPinManagement, RemoveInputPinRemovesMultipleConnections)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    core::Node &node3 = graph.CreateNode(3);
    
    core::PinId outPin1 = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId outPin2 = graph.AddOutputPin(node2.Id(), "out", 0);
    core::PinId inPin = graph.AddInputPin(node3.Id(), "in", 0);
    
    graph.Connect(node1.Id(), outPin1, node3.Id(), inPin);
    graph.Connect(node2.Id(), outPin2, node3.Id(), inPin);
    EXPECT_EQ(graph.GetAllConnections().size(), 2);
    
    graph.RemoveInputPin(node3.Id(), inPin);
    EXPECT_EQ(graph.GetAllConnections().size(), 0);
}

// ============================================================================
// Pin Management Tests (Output Pins)
// ============================================================================

TEST(GraphOutputPinManagement, AddOutputPinAddsToExistingNode)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    
    core::PinId pinId = graph.AddOutputPin(node.Id(), "out_pin", 0);
    EXPECT_GT(pinId, 0);
    EXPECT_TRUE(node.OutputPinExists(pinId));
}

TEST(GraphOutputPinManagement, AddOutputPinReturnsValidPinID)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    
    core::PinId pinId = graph.AddOutputPin(node.Id(), "pin1", 0);
    EXPECT_GT(pinId, 0);
}

TEST(GraphOutputPinManagement, AddOutputPinThrowsForInvalidNode)
{
    core::Graph graph;
    
    EXPECT_THROW(
        graph.AddOutputPin(999, "pin", 0),
        core::NodeNotFoundException
    );
}

TEST(GraphOutputPinManagement, RemoveOutputPinRemovesExisting)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    core::PinId pinId = graph.AddOutputPin(node.Id(), "pin", 0);
    
    EXPECT_TRUE(node.OutputPinExists(pinId));
    graph.RemoveOutputPin(node.Id(), pinId);
    EXPECT_FALSE(node.OutputPinExists(pinId));
}

TEST(GraphOutputPinManagement, RemoveOutputPinThrowsForInvalidNode)
{
    core::Graph graph;
    
    EXPECT_THROW(
        graph.RemoveOutputPin(999, 1),
        core::NodeNotFoundException
    );
}

TEST(GraphOutputPinManagement, RemoveOutputPinRemovesAssociatedConnections)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin = graph.AddInputPin(node2.Id(), "in", 0);
    
    graph.Connect(node1.Id(), outPin, node2.Id(), inPin);
    EXPECT_EQ(graph.GetAllConnections().size(), 1);
    
    graph.RemoveOutputPin(node1.Id(), outPin);
    EXPECT_EQ(graph.GetAllConnections().size(), 0);
}

TEST(GraphOutputPinManagement, RemoveOutputPinRemovesMultipleConnections)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    core::Node &node3 = graph.CreateNode(3);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin1 = graph.AddInputPin(node2.Id(), "in", 0);
    core::PinId inPin2 = graph.AddInputPin(node3.Id(), "in", 0);
    
    graph.Connect(node1.Id(), outPin, node2.Id(), inPin1);
    graph.Connect(node1.Id(), outPin, node3.Id(), inPin2);
    EXPECT_EQ(graph.GetAllConnections().size(), 2);
    
    graph.RemoveOutputPin(node1.Id(), outPin);
    EXPECT_EQ(graph.GetAllConnections().size(), 0);
}

// ============================================================================
// Connection Management Tests
// ============================================================================

TEST(GraphConnectionManagement, ConnectTwoPinsWithMatchingTypesSucceeds)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin = graph.AddInputPin(node2.Id(), "in", 0);
    
    core::ConnectionId connId = graph.Connect(node1.Id(), outPin, node2.Id(), inPin);
    EXPECT_GT(connId, 0);
}

TEST(GraphConnectionManagement, ConnectReturnsUniqueConnectionID)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    core::Node &node3 = graph.CreateNode(3);
    
    core::PinId outPin1 = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId outPin2 = graph.AddOutputPin(node2.Id(), "out", 0);
    core::PinId inPin1 = graph.AddInputPin(node3.Id(), "in", 0);
    core::PinId inPin2 = graph.AddInputPin(node3.Id(), "in2", 0);
    
    core::ConnectionId conn1 = graph.Connect(node1.Id(), outPin1, node3.Id(), inPin1);
    core::ConnectionId conn2 = graph.Connect(node2.Id(), outPin2, node3.Id(), inPin2);
    
    EXPECT_NE(conn1, conn2);
}

TEST(GraphConnectionManagement, ConnectReturnsExistingIdIfAlreadyConnected)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin = graph.AddInputPin(node2.Id(), "in", 0);
    
    core::ConnectionId conn1 = graph.Connect(node1.Id(), outPin, node2.Id(), inPin);
    core::ConnectionId conn2 = graph.Connect(node1.Id(), outPin, node2.Id(), inPin);
    
    EXPECT_EQ(conn1, conn2);
}

TEST(GraphConnectionManagement, ConnectThrowsForInvalidFromNode)
{
    core::Graph graph;
    core::Node &node2 = graph.CreateNode(2);
    
    core::PinId inPin = graph.AddInputPin(node2.Id(), "in", 0);
    
    EXPECT_THROW(
        graph.Connect(999, 1, node2.Id(), inPin),
        core::NodeNotFoundException
    );
}

TEST(GraphConnectionManagement, ConnectThrowsForInvalidToNode)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    
    EXPECT_THROW(
        graph.Connect(node1.Id(), outPin, 999, 1),
        core::NodeNotFoundException
    );
}

TEST(GraphConnectionManagement, ConnectThrowsForInvalidOutputPin)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    
    core::PinId inPin = graph.AddInputPin(node2.Id(), "in", 0);
    
    EXPECT_THROW(
        graph.Connect(node1.Id(), 999, node2.Id(), inPin),
        core::PinNotFoundException
    );
}

TEST(GraphConnectionManagement, ConnectThrowsForInvalidInputPin)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    
    EXPECT_THROW(
        graph.Connect(node1.Id(), outPin, node2.Id(), 999),
        core::PinNotFoundException
    );
}

TEST(GraphConnectionManagement, ConnectThrowsForTypeMismatch)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin = graph.AddInputPin(node2.Id(), "in", 1);
    
    EXPECT_THROW(
        graph.Connect(node1.Id(), outPin, node2.Id(), inPin),
        core::TypeMismatchException
    );
}

TEST(GraphConnectionManagement, DisconnectByIDRemovesConnection)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin = graph.AddInputPin(node2.Id(), "in", 0);
    
    core::ConnectionId connId = graph.Connect(node1.Id(), outPin, node2.Id(), inPin);
    EXPECT_EQ(graph.GetAllConnections().size(), 1);
    
    graph.Disconnect(connId);
    EXPECT_EQ(graph.GetAllConnections().size(), 0);
}

TEST(GraphConnectionManagement, DisconnectByPinsRemovesConnection)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin = graph.AddInputPin(node2.Id(), "in", 0);
    
    graph.Connect(node1.Id(), outPin, node2.Id(), inPin);
    EXPECT_EQ(graph.GetAllConnections().size(), 1);
    
    graph.Disconnect(node1.Id(), outPin, node2.Id(), inPin);
    EXPECT_EQ(graph.GetAllConnections().size(), 0);
}

TEST(GraphConnectionManagement, DisconnectByPinsDoesNothingForNonExistent)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin = graph.AddInputPin(node2.Id(), "in", 0);
    
    graph.Disconnect(node1.Id(), outPin, node2.Id(), inPin);
    EXPECT_EQ(graph.GetAllConnections().size(), 0);
}

TEST(GraphConnectionManagement, DisconnectOutputPinRemovesAllConnections)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    core::Node &node3 = graph.CreateNode(3);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin2 = graph.AddInputPin(node2.Id(), "in", 0);
    core::PinId inPin3 = graph.AddInputPin(node3.Id(), "in", 0);
    
    graph.Connect(node1.Id(), outPin, node2.Id(), inPin2);
    graph.Connect(node1.Id(), outPin, node3.Id(), inPin3);
    
    EXPECT_EQ(graph.GetAllConnections().size(), 2);
    graph.DisconnectOutputPin(node1.Id(), outPin);
    EXPECT_EQ(graph.GetAllConnections().size(), 0);
}

TEST(GraphConnectionManagement, DisconnectAllOutputPinReturnsCount)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    core::Node &node3 = graph.CreateNode(3);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin2 = graph.AddInputPin(node2.Id(), "in", 0);
    core::PinId inPin3 = graph.AddInputPin(node3.Id(), "in", 0);
    
    graph.Connect(node1.Id(), outPin, node2.Id(), inPin2);
    graph.Connect(node1.Id(), outPin, node3.Id(), inPin3);
    
    uint16_t count = graph.DisconnectAllOutputPin(node1.Id());
    EXPECT_EQ(count, 2);
}

TEST(GraphConnectionManagement, DisconnectInputPinRemovesAllConnections)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    core::Node &node3 = graph.CreateNode(3);
    
    core::PinId outPin1 = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId outPin2 = graph.AddOutputPin(node2.Id(), "out", 0);
    core::PinId inPin = graph.AddInputPin(node3.Id(), "in", 0);
    
    graph.Connect(node1.Id(), outPin1, node3.Id(), inPin);
    graph.Connect(node2.Id(), outPin2, node3.Id(), inPin);
    
    EXPECT_EQ(graph.GetAllConnections().size(), 2);
    graph.DisconnectInputPin(node3.Id(), inPin);
    EXPECT_EQ(graph.GetAllConnections().size(), 0);
}

TEST(GraphConnectionManagement, DisconnectAllInputPinReturnsCount)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    core::Node &node3 = graph.CreateNode(3);
    
    core::PinId outPin1 = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId outPin2 = graph.AddOutputPin(node2.Id(), "out", 0);
    core::PinId inPin = graph.AddInputPin(node3.Id(), "in", 0);
    
    graph.Connect(node1.Id(), outPin1, node3.Id(), inPin);
    graph.Connect(node2.Id(), outPin2, node3.Id(), inPin);
    
    uint16_t count = graph.DisconnectAllInputPin(node3.Id());
    EXPECT_EQ(count, 2);
}

TEST(GraphConnectionManagement, DisconnectNodeRemovesAllConnections)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    core::Node &node3 = graph.CreateNode(3);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin1 = graph.AddInputPin(node2.Id(), "in", 0);
    core::PinId inPin2 = graph.AddInputPin(node3.Id(), "in", 0);
    
    graph.Connect(node1.Id(), outPin, node2.Id(), inPin1);
    graph.Connect(node1.Id(), outPin, node3.Id(), inPin2);
    
    uint16_t count = graph.DisconnectNode(node1.Id());
    EXPECT_EQ(count, 2);
}

TEST(GraphConnectionManagement, GetConnectionIdReturnsCorrectID)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin = graph.AddInputPin(node2.Id(), "in", 0);
    
    core::ConnectionId connId = graph.Connect(node1.Id(), outPin, node2.Id(), inPin);
    core::ConnectionId retrievedId = graph.GetConnectionId(node1.Id(), outPin, node2.Id(), inPin);
    
    EXPECT_EQ(connId, retrievedId);
}

TEST(GraphConnectionManagement, GetConnectionIdReturnsZeroForNonExistent)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    
    core::PinId outPin = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId inPin = graph.AddInputPin(node2.Id(), "in", 0);
    
    core::ConnectionId retrievedId = graph.GetConnectionId(node1.Id(), outPin, node2.Id(), inPin);
    EXPECT_EQ(retrievedId, 0);
}

TEST(GraphConnectionManagement, GetAllConnectionsReturnsEmptyInitially)
{
    core::Graph graph;
    EXPECT_EQ(graph.GetAllConnections().size(), 0);
}

TEST(GraphConnectionManagement, GetAllConnectionsReturnsAllConnections)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    core::Node &node3 = graph.CreateNode(3);
    
    core::PinId outPin1 = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId outPin2 = graph.AddOutputPin(node2.Id(), "out", 0);
    core::PinId inPin1 = graph.AddInputPin(node3.Id(), "in1", 0);
    core::PinId inPin2 = graph.AddInputPin(node3.Id(), "in2", 0);
    
    graph.Connect(node1.Id(), outPin1, node3.Id(), inPin1);
    graph.Connect(node2.Id(), outPin2, node3.Id(), inPin2);
    
    EXPECT_EQ(graph.GetAllConnections().size(), 2);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(GraphIntegration, CreateChainOfNodes)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    core::Node &node3 = graph.CreateNode(3);
    
    core::PinId out1 = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId in1 = graph.AddInputPin(node2.Id(), "in", 0);
    core::PinId out2 = graph.AddOutputPin(node2.Id(), "out", 0);
    core::PinId in2 = graph.AddInputPin(node3.Id(), "in", 0);
    
    graph.Connect(node1.Id(), out1, node2.Id(), in1);
    graph.Connect(node2.Id(), out2, node3.Id(), in2);
    
    EXPECT_EQ(graph.GetAllConnections().size(), 2);
}

TEST(GraphIntegration, CreateBranchingConnections)
{
    core::Graph graph;
    core::Node &source = graph.CreateNode(1);
    core::Node &dest1 = graph.CreateNode(2);
    core::Node &dest2 = graph.CreateNode(3);
    
    core::PinId outPin = graph.AddOutputPin(source.Id(), "out", 0);
    core::PinId inPin1 = graph.AddInputPin(dest1.Id(), "in", 0);
    core::PinId inPin2 = graph.AddInputPin(dest2.Id(), "in", 0);
    
    graph.Connect(source.Id(), outPin, dest1.Id(), inPin1);
    graph.Connect(source.Id(), outPin, dest2.Id(), inPin2);
    
    EXPECT_EQ(graph.GetAllConnections().size(), 2);
}

TEST(GraphIntegration, CreateDiamondPattern)
{
    core::Graph graph;
    core::Node &top = graph.CreateNode(1);
    core::Node &left = graph.CreateNode(2);
    core::Node &right = graph.CreateNode(3);
    core::Node &bottom = graph.CreateNode(4);
    
    core::PinId topOut = graph.AddOutputPin(top.Id(), "out", 0);
    core::PinId leftIn = graph.AddInputPin(left.Id(), "in", 0);
    core::PinId leftOut = graph.AddOutputPin(left.Id(), "out", 0);
    core::PinId rightIn = graph.AddInputPin(right.Id(), "in", 0);
    core::PinId rightOut = graph.AddOutputPin(right.Id(), "out", 0);
    core::PinId bottomIn = graph.AddInputPin(bottom.Id(), "in", 0);
    
    graph.Connect(top.Id(), topOut, left.Id(), leftIn);
    graph.Connect(top.Id(), topOut, right.Id(), rightIn);
    graph.Connect(left.Id(), leftOut, bottom.Id(), bottomIn);
    graph.Connect(right.Id(), rightOut, bottom.Id(), bottomIn);
    
    EXPECT_EQ(graph.GetAllConnections().size(), 4);
}

TEST(GraphIntegration, RemoveNodeWithComplexConnections)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::Node &node2 = graph.CreateNode(2);
    core::Node &node3 = graph.CreateNode(3);
    
    core::PinId out1 = graph.AddOutputPin(node1.Id(), "out", 0);
    core::PinId in2 = graph.AddInputPin(node2.Id(), "in", 0);
    core::PinId out2 = graph.AddOutputPin(node2.Id(), "out", 0);
    core::PinId in3 = graph.AddInputPin(node3.Id(), "in", 0);
    
    graph.Connect(node1.Id(), out1, node2.Id(), in2);
    graph.Connect(node2.Id(), out2, node3.Id(), in3);
    
    graph.RemoveNode(node2.Id());
    
    EXPECT_FALSE(graph.HasNode(node2.Id()));
    EXPECT_EQ(graph.GetAllConnections().size(), 0);
}

TEST(GraphIntegration, AddRemovePinsDuringNodeLifetime)
{
    core::Graph graph;
    core::Node &node = graph.CreateNode(1);
    
    core::PinId pin1 = graph.AddInputPin(node.Id(), "pin1", 0);
    EXPECT_EQ(node.InputPinsCount(), 1);
    
    core::PinId pin2 = graph.AddInputPin(node.Id(), "pin2", 0);
    EXPECT_EQ(node.InputPinsCount(), 2);
    
    graph.RemoveInputPin(node.Id(), pin1);
    EXPECT_EQ(node.InputPinsCount(), 1);
    
    graph.RemoveInputPin(node.Id(), pin2);
    EXPECT_EQ(node.InputPinsCount(), 0);
}

TEST(GraphIntegration, OperationsAfterNodeRemovalDontCrash)
{
    core::Graph graph;
    core::Node &node1 = graph.CreateNode(1);
    core::NodeId id1 = node1.Id();
    
    graph.RemoveNode(id1);
    
    // These should not crash
    EXPECT_FALSE(graph.HasNode(id1));
    EXPECT_EQ(graph.GetNode(id1), nullptr);
}
