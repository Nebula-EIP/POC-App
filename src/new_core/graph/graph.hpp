/**
 * @file graph.hpp
 * @brief The graph is a collection of nodes connected together
 *
 * It is oblivious to the nodes types or their datas.
 * This is handled by the modules directly.
 *
 * @author Created by JeanBizeul
 * @date Created on 01-08-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 10-09-2026
 */

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "datatypes.hpp"
#include "node.hpp"

namespace core {

/**
 * @brief Collection of linked nodes.
 *
 * The graph handles the lifetime of the nodes and their connections.
 * It is a master to slave relationship.
 */
class Graph {
   public:
    Graph();
    ~Graph() = default;

    /**
     * @brief Checks if a node exists in the graph.
     *
     * @param id Id of the node. Required
     *
     * @return true if the node exists, false if not
     */
    bool hasNode(NodeId id) const noexcept;

    /**
     * @brief Try to retreive a pointer to a node.
     *
     * @param id Id of the node. Required
     *
     * @return A pointer to the node if found, nullptr if not.
     */
    Node *node(NodeId id) noexcept;

    /**
     * @brief Try to retreive a constant pointer to a node.
     *
     * @param id Id of the node. Required
     *
     * @return A constant pointer to the node if found, nullptr if not.
     */
    const Node *node(NodeId id) const noexcept;

    /**
     * @brief Retreive the list of all nodes.
     *
     * @return All nodes contained in the graph.
     */
    const std::unordered_map<NodeId, Node> &nodes() const noexcept;

    /**
     * @brief Add a new node to the graph
     *
     * @param type Type of the node
     *
     * @return The created node.
     */
    Node &CreateNode(NodeType type);

    /**
     * @brief Remove a node from the graph
     *
     * @param id Id of the node to remove. Required
     *
     * @return True if the node has been removed.
     * False if the node is not in the graph.
     */
    bool RemoveNode(NodeId id);

    /**
     * @brief Add an input pin to a node
     *
     * @param node_id Id of the targeted node. Required
     * @param name Name of the pin. Required
     * @param pin_type Type of the input pin. Required
     *
     * @returns The id of the added pin
     *
     * @throws `core::NodeNotFoundException` The node_id is invalid.
     */
    PinId AddInputPin(NodeId node_id, const std::string_view &name,
                      DataType in_type);

    /**
     * @brief Add an output pin to a node
     *
     * @param node_id Id of the targeted node. Required
     * @param name Name of the pin. Required
     * @param pin_id Type of the output pin. Required
     *
     * @returns The id of the added pin
     *
     * @throws `core::NodeNotFoundException` The node_id is invalid.
     */
    PinId AddOutputPin(NodeId node_id, const std::string_view &name,
                       DataType in_type);

    /**
     * @brief Remove an input pin from a node
     *
     * @param node_id Id of the targeted node. Required
     * @param pin_id Input pin id of said node. Required
     *
     * @return true if the pin has been deleted, false if it has not been
     *
     * @throws `core::NodeNotFoundException` The node_id is invalid.
     *
     * @warning This method disconnects all connection to this pin !
     */
    void RemoveInputPin(NodeId node_id, PinId pin_id);

    /**
     * @brief Remove an output pin from a node
     *
     * @param node_id Id of the targeted node. Required
     * @param pin_id Output pin id of said node. Required
     *
     * @return true if the pin has been deleted, false if it has not been
     *
     * @throws `core::NodeNotFoundException` The node_id is invalid.
     *
     * @warning This method disconnects all connection to this pin !
     */
    void RemoveOutputPin(NodeId node_id, PinId pin_id);

    /**
     * @brief Retreive the id of a connection between two pins
     *
     * @param from Node owning the output pin
     * @param out Output pin
     * @param to Node owning the input pin
     * @param in Input pin
     *
     * @return 0 if connection not found, otherwise the connection id
     */
    ConnectionId getConnectionId(NodeId from, PinId out, NodeId to,
                                 PinId in) const;

    /**
     * @brief Retreive a read only map of all connections in the graph with
     * their id as key
     *
     * @returns A map containing all the connections between
     * the nodes of the graph with their ids as key
     */
    const std::unordered_map<ConnectionId, Connection> &getAllConnections()
        const noexcept;

    /**
     * @brief Connects two pins
     *
     * Only an output pin can be connected to an input pin and
     * vice-versa.
     *
     * The pins types must match.
     *
     * @param from Node owning the output pin
     * @param out Output pin
     * @param to Node owning the input pin
     * @param in Input pin
     *
     * @return The id of the new connection
     *
     * @throws `core::NodeNotFoundException` when a node does not exists
     * @throws `core::PinNotFoundException` when a pin does not exists
     * @throws `core::TypeMismatchException` when the pin's types mismatches
     */
    ConnectionId Connect(NodeId from, PinId out, NodeId to, PinId in);

    /**
     * @brief Delete a connection by ConnectionId
     *
     * @param id Connection id
     */
    void Disconnect(ConnectionId id);

    /**
     * @brief Disconnects two pins by start/end pos
     *
     * @param from Node owning the output pin
     * @param out Output pin
     * @param to Node owning the input pin
     * @param in Input pin
     *
     * @warning This method delete a connection between two pins
     */
    void Disconnect(NodeId from, PinId out, NodeId to, PinId in);

    /**
     * @brief Disconnects all connection from an output pin
     *
     * @param node_id Id of the node
     * @param pin_id Id of the pin
     */
    void DisconnectOutputPin(NodeId node_id, PinId pin_id);

    /**
     * @brief Disconnects all connection outgoing from a node
     *
     * @param node_id Id of the node
     *
     * @return Amount of connections severed
     */
    uint16_t DisconnectAllOutputPin(NodeId id);

    /**
     * @brief Disconnects all connection to an input pin
     *
     * @param node_id Id of the node
     * @param pin_id Id of the pin
     */
    void DisconnectInputPin(NodeId node_id, PinId pin_id);

    /**
     * @brief Disconnects all connection incoming to a node
     *
     * @param node_id Id of the node
     *
     * @return Amount of connections severed
     */
    uint16_t DisconnectAllInputPin(NodeId id);

    /**
     * @brief Disconnects all connections to/from a node
     *
     * @param id Id of the node
     */
    uint16_t DisconnectNode(NodeId id);

   private:
    NodeId _next_node_id;
    std::unordered_map<NodeId, Node>
        _nodes;  ///< Map of all nodes stored in the graph

    ConnectionId _next_connection_id;
    std::unordered_map<ConnectionId, Connection>
        _connections;  ///< List of all connections linking nodes in the graph
};

}  // namespace core
