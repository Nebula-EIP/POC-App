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
#include "exception/graph_exception/node_exception.hpp"
#include "exception/graph_exception/connection_exception.hpp"

#include <algorithm>

namespace core {

Graph::Graph()
    : _next_node_id(1), _next_connection_id(1)
{}

#pragma region Nodes

bool Graph::hasNode(NodeId id) const noexcept
{
    return _nodes.find(id) != _nodes.end();
}

Node *Graph::node(NodeId id) noexcept
{
    auto node_pos = _nodes.find(id);

    if (node_pos == _nodes.end())
        return nullptr;
    else
        return &node_pos->second;
}

const Node *Graph::node(NodeId id) const noexcept
{
    const auto node_pos = _nodes.find(id);

    if (node_pos == _nodes.end())
        return nullptr;
    else
        return &node_pos->second;
}

const std::unordered_map<NodeId, Node> &Graph::nodes() const noexcept
{
    return _nodes;
}

Node &Graph::CreateNode(core::NodeType type)
{
    Node node(_next_node_id++, type);

    _nodes.emplace(node.id(), std::move(node));
    return _nodes.at(node.id());
}

bool Graph::RemoveNode(NodeId id)
{
    if (_nodes.erase(id) == 1)
        return true;
    else
        return false;
}

#pragma endregion Nodes

#pragma region Pins

PinId Graph::AddInputPin(NodeId node_id, const std::string_view &name, DataType type)
{
    if (!hasNode(node_id))
        throw core::NodeNotFoundException("Node not found in the graph");

    Pin pin;
    pin.id = 0;
    pin.name = name;
    pin.type = type;

    return _nodes.at(node_id).AddInputPin(pin);
    /// No need to try & catch here as the existence of the node is proven previously
}

PinId Graph::AddOutputPin(NodeId node_id, const std::string_view &name, DataType type)
{
    if (!hasNode(node_id))
        throw core::NodeNotFoundException("Node not found in the graph");

    Pin pin;
    pin.id = 0;
    pin.name = name;
    pin.type = type;

    return _nodes.at(node_id).AddOutputPin(pin);
    /// No need to try & catch here as the existence of the node is proven previously
}

void Graph::RemoveInputPin(NodeId node_id, PinId pin_id)
{
    if (!hasNode(node_id))
        throw core::NodeNotFoundException("Node not found in the graph");

    _nodes.at(node_id).RemoveInputPin(pin_id);
    /// Need to use at bcause no empty constructor plus already checked it exists
}

void Graph::RemoveOutputPin(NodeId node_id, PinId pin_id)
{
    if (!hasNode(node_id))
        throw core::NodeNotFoundException("Node not found in the graph");

    _nodes.at(node_id).RemoveOutputPin(pin_id);
    /// Same as last method
}

#pragma endregion Pins

#pragma region Connections

ConnectionId Graph::getConnectionId(NodeId from, PinId out, NodeId to, PinId in) const
{
    auto res = std::find_if(_connections.begin(), _connections.end(),
        [from, out, to, in](const std::pair<ConnectionId, Connection> &connection){
            if (connection.second.from_node != from) return false;
            if (connection.second.to_node != to) return false;
            if (connection.second.out_pin != out) return false;
            if (connection.second.in_pin != in) return false;

            return true;
        });

    if (res == _connections.end())
        return 0;
    else
        return res->first;
}

const std::unordered_map<ConnectionId, Connection> &Graph::getAllConnections() const noexcept
{
    return _connections;
}

ConnectionId Graph::Connect(
    NodeId from_node_id, PinId out_pin_id, NodeId to_node_id, PinId in_pin_id)
{
    ConnectionId conn_id = getConnectionId(from_node_id, out_pin_id, to_node_id, in_pin_id);
    if (conn_id != 0)
        return conn_id;

    if (!hasNode(from_node_id))
        throw NodeNotFoundException("From node not found");

    if (!hasNode(to_node_id))
        throw NodeNotFoundException("To node not found");

    Node *from_node = &(_nodes.at(from_node_id));
    Node *to_node = &(_nodes.at(to_node_id));

    const Pin *out_pin = from_node->outputPin(out_pin_id);
    const Pin *in_pin = to_node->inputPin(in_pin_id);

    if (out_pin == nullptr)
        throw PinNotFoundException("Output pin not found");

    if (in_pin == nullptr)
        throw PinNotFoundException("Input pin not found");

    if (_nodes.at(from_node_id).outputPin(out_pin_id)->type
        != _nodes.at(to_node_id).inputPin(in_pin_id)->type)
        throw TypeMismatchException("Pins types does not match");

    Connection connection;
    connection.data_type = out_pin->type;
    connection.from_node = from_node->id();
    connection.to_node = to_node->id();
    connection.out_pin = out_pin->id;
    connection.in_pin = in_pin->id;

    conn_id = _next_connection_id++;
    _connections.insert_or_assign(conn_id, connection);
    return conn_id;
}

void Graph::Disconnect(ConnectionId id)
{
    _connections.erase(id);
}

void Graph::Disconnect(NodeId from, PinId out, NodeId to, PinId in)
{
    ConnectionId conn_id = getConnectionId(from, out, to, in);

    if (conn_id != 0)
        Disconnect(conn_id);
}

void Graph::DisconnectOutputPin(NodeId node_id, PinId pin_id)
{
    bool found = true;

    while (found) {
        found = false;
        auto conn_pos = std::find_if(_connections.begin(), _connections.end(),
            [node_id, pin_id](const std::pair<ConnectionId, Connection> &conn)
            {
                if (conn.second.from_node == node_id && conn.second.out_pin == pin_id)
                    return true;
                else
                    return false;
            });

        if (conn_pos != _connections.end()) {
            found = true;
            const Connection &conn = conn_pos->second;
            Disconnect(conn.from_node, conn.out_pin, conn.to_node, conn.in_pin);
        }
    }
}

uint16_t Graph::DisconnectAllOutputPin(NodeId node_id)
{
    uint16_t count = 0;
    bool found = true;

    while (found) {
        found = false;
        auto conn_pos = std::find_if(_connections.begin(), _connections.end(),
            [node_id](const std::pair<ConnectionId, Connection> &conn)
            {
                if (conn.second.from_node == node_id)
                    return true;
                else
                    return false;
            });

        if (conn_pos != _connections.end()) {
            found = true;
            const Connection &conn = conn_pos->second;
            Disconnect(conn.from_node, conn.out_pin, conn.to_node, conn.in_pin);
            count++;
        }
    }
    return count;
}

void Graph::DisconnectInputPin(NodeId node_id, PinId pin_id)
{
    bool found = true;

    while (found) {
        found = false;
        auto conn_pos = std::find_if(_connections.begin(), _connections.end(),
            [node_id, pin_id](const std::pair<ConnectionId, Connection> &conn)
            {
                if (conn.second.to_node == node_id && conn.second.in_pin == pin_id)
                    return true;
                else
                    return false;
            });

        if (conn_pos != _connections.end()) {
            found = true;
            const Connection &conn = conn_pos->second;
            Disconnect(conn.from_node, conn.out_pin, conn.to_node, conn.in_pin);
        }
    }
}

uint16_t Graph::DisconnectAllInputPin(NodeId node_id)
{
    uint16_t count = 0;
    bool found = true;

    while (found) {
        found = false;
        auto conn_pos = std::find_if(_connections.begin(), _connections.end(),
            [node_id](const std::pair<ConnectionId, Connection> &conn)
            {
                if (conn.second.to_node == node_id)
                    return true;
                else
                    return false;
            });

        if (conn_pos != _connections.end()) {
            found = true;
            const Connection &conn = conn_pos->second;
            Disconnect(conn.from_node, conn.out_pin, conn.to_node, conn.in_pin);
            count++;
        }
    }
    return count;
}

uint16_t Graph::DisconnectNode(NodeId id)
{
    return (DisconnectAllOutputPin(id) + DisconnectAllInputPin(id));
}

#pragma endregion Connections

} // namespace core
