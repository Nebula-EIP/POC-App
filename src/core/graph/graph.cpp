/**
 * @file graph.cpp
 * @brief Implementation of the graph class
 *
 * @author Created by JeanBizeul
 * @date Created on 08-09-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 08-09-2026
 */

#include "graph.hpp"

#include <algorithm>

#include "exception/graph_exception/connection_exception.hpp"
#include "exception/graph_exception/node_exception.hpp"

namespace core {

Graph::Graph() : next_node_id_(1), next_connection_id_(1) {}

#pragma region Nodes

bool Graph::HasNode(NodeId id) const noexcept {
    return nodes_.find(id) != nodes_.end();
}

Node *Graph::GetNode(NodeId id) noexcept {
    auto node_pos = nodes_.find(id);

    if (node_pos == nodes_.end()) {
        return nullptr;
    } else {
        return &node_pos->second;
    }
}

const Node *Graph::GetNode(NodeId id) const noexcept {
    const auto kNodePos = nodes_.find(id);

    if (kNodePos == nodes_.end()) {
        return nullptr;
    } else {
        return &kNodePos->second;
    }
}

const std::unordered_map<NodeId, Node> &Graph::GetAllNodes() const noexcept {
    return nodes_;
}

Node &Graph::CreateNode(core::NodeType type) {
    Node node(next_node_id_++, type);

    nodes_.emplace(node.Id(), std::move(node));
    return nodes_.at(node.Id());
}

bool Graph::RemoveNode(NodeId id) {
    if (!HasNode(id)) {
        return false;
    }

    DisconnectNode(id);
    if (nodes_.erase(id)) {
        return true;
    } else {
        return false;
    }
}

#pragma endregion Nodes

#pragma region Pins

PinId Graph::AddInputPin(NodeId node_id, const std::string_view &name,
                         DataType type) {
    if (!HasNode(node_id)) {
        throw core::NodeNotFoundException("Node not found in the graph");
    }

    Pin pin;
    pin.id_ = 0;
    pin.name_ = name;
    pin.type_ = type;

    return nodes_.at(node_id).AddInputPin(pin);
    /// No need to try & catch here as the existence of the node is proven
    /// previously
}

PinId Graph::AddOutputPin(NodeId node_id, const std::string_view &name,
                          DataType type) {
    if (!HasNode(node_id)) {
        throw core::NodeNotFoundException("Node not found in the graph");
    }

    Pin pin;
    pin.id_ = 0;
    pin.name_ = name;
    pin.type_ = type;

    return nodes_.at(node_id).AddOutputPin(pin);
    /// No need to try & catch here as the existence of the node is proven
    /// previously
}

void Graph::RemoveInputPin(NodeId node_id, PinId pin_id) {
    if (!HasNode(node_id)) {
        throw core::NodeNotFoundException("Node not found in the graph");
    }

    DisconnectInputPin(node_id, pin_id);
    nodes_.at(node_id).RemoveInputPin(pin_id);
    /// Need to use at bcause no empty constructor plus already checked it
    /// exists
}

void Graph::RemoveOutputPin(NodeId node_id, PinId pin_id) {
    if (!HasNode(node_id)) {
        throw core::NodeNotFoundException("Node not found in the graph");
    }

    DisconnectOutputPin(node_id, pin_id);
    nodes_.at(node_id).RemoveOutputPin(pin_id);
    /// Same as last method
}

#pragma endregion Pins

#pragma region Connections

ConnectionId Graph::GetConnectionId(NodeId from, PinId out, NodeId to,
                                    PinId in) const {
    auto res = std::find_if(
        connections_.begin(), connections_.end(),
        [from, out, to,
         in](const std::pair<ConnectionId, Connection> &connection) {
            if (connection.second.from_node_ != from) return false;
            if (connection.second.to_node_ != to) return false;
            if (connection.second.out_pin_ != out) return false;
            if (connection.second.in_pin_ != in) return false;

            return true;
        });

    if (res == connections_.end()) {
        return 0;
    } else {
        return res->first;
    }
}

const std::unordered_map<ConnectionId, Connection> &Graph::GetAllConnections()
    const noexcept {
    return connections_;
}

ConnectionId Graph::Connect(NodeId from_node_id, PinId out_pin_id,
                            NodeId to_node_id, PinId in_pin_id) {
    ConnectionId conn_id =
        GetConnectionId(from_node_id, out_pin_id, to_node_id, in_pin_id);
    if (conn_id != 0) {
        return conn_id;
    }

    if (!HasNode(from_node_id)) {
        throw NodeNotFoundException("From node not found");
    }

    if (!HasNode(to_node_id)) {
        throw NodeNotFoundException("To node not found");
    }

    Node *from_node = &(nodes_.at(from_node_id));
    Node *to_node = &(nodes_.at(to_node_id));

    const Pin *out_pin = from_node->OutputPin(out_pin_id);
    const Pin *in_pin = to_node->InputPin(in_pin_id);

    if (out_pin == nullptr) {
        throw PinNotFoundException("Output pin not found");
    }

    if (in_pin == nullptr) {
        throw PinNotFoundException("Input pin not found");
    }

    if (nodes_.at(from_node_id).OutputPin(out_pin_id)->type_ !=
        nodes_.at(to_node_id).InputPin(in_pin_id)->type_) {
        throw TypeMismatchException("Pins types does not match");
    }

    Connection connection;
    connection.data_type_ = out_pin->type_;
    connection.from_node_ = from_node->Id();
    connection.to_node_ = to_node->Id();
    connection.out_pin_ = out_pin->id_;
    connection.in_pin_ = in_pin->id_;

    conn_id = next_connection_id_++;
    connections_.insert_or_assign(conn_id, connection);
    return conn_id;
}

void Graph::Disconnect(ConnectionId id) { connections_.erase(id); }

void Graph::Disconnect(NodeId from, PinId out, NodeId to, PinId in) {
    ConnectionId conn_id = GetConnectionId(from, out, to, in);

    if (conn_id != 0) {
        Disconnect(conn_id);
    }
}

void Graph::DisconnectOutputPin(NodeId node_id, PinId pin_id) {
    bool found = true;

    while (found) {
        found = false;
        auto conn_pos = std::find_if(
            connections_.begin(), connections_.end(),
            [node_id, pin_id](const std::pair<ConnectionId, Connection> &conn) {
                if (conn.second.from_node_ == node_id &&
                    conn.second.out_pin_ == pin_id) {
                    return true;
                } else {
                    return false;
                }
            });

        if (conn_pos != connections_.end()) {
            found = true;
            const Connection &conn = conn_pos->second;
            Disconnect(conn.from_node_, conn.out_pin_, conn.to_node_, conn.in_pin_);
        }
    }
}

uint16_t Graph::DisconnectAllOutputPin(NodeId node_id) {
    uint16_t count = 0;
    bool found = true;

    while (found) {
        found = false;
        auto conn_pos = std::find_if(
            connections_.begin(), connections_.end(),
            [node_id](const std::pair<ConnectionId, Connection> &conn) {
                if (conn.second.from_node_ == node_id) {
                    return true;
                } else {
                    return false;
                }
            });

        if (conn_pos != connections_.end()) {
            found = true;
            const Connection &conn = conn_pos->second;
            Disconnect(conn.from_node_, conn.out_pin_, conn.to_node_, conn.in_pin_);
            count++;
        }
    }
    return count;
}

void Graph::DisconnectInputPin(NodeId node_id, PinId pin_id) {
    bool found = true;

    while (found) {
        found = false;
        auto conn_pos = std::find_if(
            connections_.begin(), connections_.end(),
            [node_id, pin_id](const std::pair<ConnectionId, Connection> &conn) {
                if (conn.second.to_node_ == node_id &&
                    conn.second.in_pin_ == pin_id) {
                    return true;
                } else {
                    return false;
                }
            });

        if (conn_pos != connections_.end()) {
            found = true;
            const Connection &conn = conn_pos->second;
            Disconnect(conn.from_node_, conn.out_pin_, conn.to_node_, conn.in_pin_);
        }
    }
}

uint16_t Graph::DisconnectAllInputPin(NodeId node_id) {
    uint16_t count = 0;
    bool found = true;

    while (found) {
        found = false;
        auto conn_pos = std::find_if(
            connections_.begin(), connections_.end(),
            [node_id](const std::pair<ConnectionId, Connection> &conn) {
                if (conn.second.to_node_ == node_id) {
                    return true;
                } else {
                    return false;
                }
            });

        if (conn_pos != connections_.end()) {
            found = true;
            const Connection &conn = conn_pos->second;
            Disconnect(conn.from_node_, conn.out_pin_, conn.to_node_, conn.in_pin_);
            count++;
        }
    }
    return count;
}

uint16_t Graph::DisconnectNode(NodeId id) {
    return (DisconnectAllOutputPin(id) + DisconnectAllInputPin(id));
}

#pragma endregion Connections

}  // namespace core
