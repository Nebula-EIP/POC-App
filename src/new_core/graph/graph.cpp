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

#include <algorithm>

namespace core {

Graph::Graph()
    : _next_node_id(1)
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

NodeId Graph::AddNode(Node node)
{
    node._id = _next_node_id++; /// Only instance when this value is edited is in this method

    _nodes.emplace(node.id(), node);
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
    if (_nodes.find(node_id) == _nodes.end())
        throw core::NodeNotFoundException("Node not found in the graph");

    Pin pin;
    pin.id = 0;
    pin.name = name;
    pin.type = type;

    return _nodes[node_id].AddInputPin(pin);
}

PinId Graph::AddOutputPin(NodeId node_id, const std::string_view &name, DataType type)
{
    if (_nodes.find(node_id) == _nodes.end())
        throw core::NodeNotFoundException("Node not found in the graph");

    Pin pin;
    pin.id = 0;
    pin.name = name;
    pin.type = type;

    return _nodes[node_id].AddOutputPin(pin);
}

void Graph::RemoveInputPin(NodeId node_id, PinId pin_id)
{
    if (_nodes.find(node_id) == _nodes.end())
        throw core::NodeNotFoundException("Node not found in the graph");

    _nodes[node_id].RemoveInputPin(pin_id);
}

void Graph::RemoveOutputPin(NodeId node_id, PinId pin_id)
{
    if (_nodes.find(node_id) == _nodes.end())
        throw core::NodeNotFoundException("Node not found in the graph");

    _nodes[node_id].RemoveOutputPin(pin_id);
}

#pragma endregion Pins

#pragma region Connections

bool Graph::hasConnection(NodeId from, PinId out, NodeId to, PinId in) const
{
    auto res = std::find(_connections.begin(), _connections.end(),
        [from, out, to, in](const Connection &connection){
            if (connection.from_node != from) return false;
            if (connection.to_node != to) return false;
            if (connection.out_pin != out) return false;
            if (connection.in_pin != in) return false;

            return true;
        });

    if (res == _connections.end())
        return false;
    else
        return true;
}

const std::vector<Connection> &Graph::getAllConnections() const noexcept
{
    return _connections;
}

void Graph::Connect(NodeId from, PinId out, NodeId to, PinId in)
{

}

#pragma endregion Connections

} // namespace core
