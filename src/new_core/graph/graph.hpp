#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "datatypes.hpp"
#include "node.hpp"

namespace core {

/**
 * @brief Node graph manager.
 */
class Graph {
   public:
    Graph();
    ~Graph();

    bool hasNode(NodeId id) const;
    Node *node(NodeId id);
    const Node *node(NodeId id) const;
    const std::unordered_map<NodeId, Node> &nodes() const noexcept;
    /**
     * @warning Will overwrite the node id and return the new one
     */
    NodeId AddNode(Node node);
    bool RemoveNode(NodeId id);

    /**
     * @returns false if the pin id is already taken or if the max amount of
     * pins has been reached
     */
    bool AddInputPin(NodeId node_id, Pin pin_id);
    /**
     * @returns false if the pin id is already taken or if the max amount of
     * pins has been reached
     */
    bool AddOutputPin(NodeId node_id, Pin pin_id);

    bool RemoveInputPin(NodeId node_id, PinId pin_id);
    bool RemoveOutputPin(NodeId node_id, PinId pin_id);

    bool hasSubgraph(NodeId id) const;
    Graph *subgraph(NodeId id);
    const Graph *subgraph(NodeId id) const;

    bool hasConnection(NodeId from, PinId out, NodeId to, PinId in) const;
    /**
     * @returns A pair of vector of connection,
     * first vector has incoming connections and second has outgoing
     * connections.
     */
    std::pair<std::vector<const Connection *>, std::vector<const Connection *>>
    getConnections(NodeId id);
    const std::vector<Connection> &getAllConnections() const;
    bool Connect(NodeId from, PinId out, NodeId to, PinId in);
    bool Disconnect(NodeId from, PinId out, NodeId to, PinId in);

   private:
    std::unordered_map<NodeId, Node> _nodes;
    std::vector<Connection> _connections;
    std::unordered_map<NodeId, std::unique_ptr<Graph>> _subgraphs;
};

}  // namespace core
