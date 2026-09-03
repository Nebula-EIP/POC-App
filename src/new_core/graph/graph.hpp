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
 * @date Last modified on 26-08-2026
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
    ~Graph();

    /**
     * @brief Checks if a node exists in the graph.
     *
     * @param id Id of the node. Required
     *
     * @return true if the node exists, false if not
     */
    bool hasNode(NodeId id) const;

    /**
     * @brief Try to retreive a pointer to a node.
     *
     * @param id Id of the node. Required
     *
     * @return A pointer to the node if found, nullptr if not.
     */
    Node *node(NodeId id);

    /**
     * @brief Try to retreive a constant pointer to a node.
     *
     * @param id Id of the node. Required
     *
     * @return A constant pointer to the node if found, nullptr if not.
     */
    const Node *node(NodeId id) const;

    /**
     * @brief Retreive the list of all nodes.
     *
     * @return All nodes contained in the graph.
     */
    const std::unordered_map<NodeId, Node> &nodes() const noexcept;

    /**
     * @brief Add a new node to the graph
     *
     * @param node Node to be added. Required
     *
     * @return The id attributed to the node.
     *
     * @warning Will overwrite the node id and return the new one
     */
    NodeId AddNode(Node node);

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
     * @param pin_type Type of the input pin. Required
     *
     *
     * @returns 0 if:
     *
     * - The node_id is invalid.
     *
     * - There is no more space in the node.
     */
    PinId AddInputPin(NodeId node_id, Property pin_type);

    /**
     * @brief Add an output pin to a node
     *
     * @param node_id Id of the targeted node. Required
     * @param pin_id Type of the output pin. Required
     *
     * @returns 0 if:
     *
     * - The node_id is invalid.
     *
     * - There is no more space in the node.
     */
    PinId AddOutputPin(NodeId node_id, Property pin_type);

    /**
     * @brief Remove an input pin from a node
     *
     * @param node_id Id of the targeted node. Required
     * @param pin_id Input pin id of said node. Required
     *
     * @return true if the pin has been deleted, false if it has not been
     *
     * @warning This method disconnects all connection to this pin !
     */
    bool RemoveInputPin(NodeId node_id, PinId pin_id);

    /**
     * @brief Remove an output pin from a node
     *
     * @param node_id Id of the targeted node. Required
     * @param pin_id Output pin id of said node. Required
     *
     * @return true if the pin has been deleted, false if it has not been
     *
     * @warning This method disconnects all connection from this pin !
     */
    bool RemoveOutputPin(NodeId node_id, PinId pin_id);

    /**
     * @brief Checks if a connection exists between two pins
     *
     * @param from Node owning the output pin
     * @param out Output pin
     * @param to Node owning the input pin
     * @param in Input pin
     *
     * @return true if a connection exists, false if not
     */
    bool hasConnection(NodeId from, PinId out, NodeId to, PinId in) const;

    /**
     * @brief Retreive a list of all connections in the graph
     *
     * @returns A vector containing all the conenctions between the nodes of the
     * graph
     */
    const std::vector<Connection> &getAllConnections() const;

    /**
     * @brief Connects two pins
     *
     * @param from Node owning the output pin
     * @param out Output pin
     * @param to Node owning the input pin
     * @param in Input pin
     *
     * @return true if the connection has been established, false if it does not
     * exists.
     *
     * @warning Only an output pin can be connected to an input pin and
     * vice-versa
     */
    bool Connect(NodeId from, PinId out, NodeId to, PinId in);

    /**
     * @brief Disconnects two pins
     *
     * @param from Node owning the output pin
     * @param out Output pin
     * @param to Node owning the input pin
     * @param in Input pin
     *
     * @return true if the nodes have been disconnected, false if they were not.
     *
     * @warning This method disconnects all connections from/to these two pins
     */
    bool Disconnect(NodeId from, PinId out, NodeId to, PinId in);

   private:
    std::unordered_map<NodeId, Node>
        _nodes;  ///< Map of all nodes stored in the graph
    std::vector<Connection>
        _connections;  ///< List of all connections linking nodes in the graph
};

}  // namespace core
