/**
 * This version of the Nebula Node Graph V0.
 * Purposely built to handle basic C without includes nor macros.
 */

#pragma once
#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <vector>

#include "node_base.hpp"

namespace core {

class Graph {
   public:
    Graph() = default;
    ~Graph() = default;

    /**
     * @brief Adds a new node of the specified kind to the graph.
     *
     * @param kind The kind of the node to be added, specified as a
     * NodeBase::NodeKind.
     * @return A pointer to the newly added node as NodeBase*.
     *
     * @note Prefer to use the templated version if you want to get back the
     * derived class.
     */
    NodeBase *AddNode(NodeBase::NodeKind kind);

    /**
     * @brief Adds a new node of the specified kind to the graph with type
     * safety.
     *
     * @tparam T The type of the node to be added. This should derive from
     * NodeBase.
     * @param kind The kind of the node to be added, specified as a
     * NodeBase::NodeKind.
     * @return A pointer to the newly added node of type T.
     *
     * @note The caller is responsible for ensuring that the type T is
     * compatible with the specified NodeKind. Misuse will result in a
     * compilation error.
     */
    template <typename T>
    T *AddNode(NodeBase::NodeKind kind);

    /**
     * @brief Removes a node from the graph.
     *
     * @param node A pointer to the node to be removed.
     *
     * @note The caller is responsible for ensuring that the node pointer is
     * valid and belongs to this graph. Removing a node will also unlink it from
     *       any connected nodes.
     */
    void RemoveNode(NodeBase *node);

    /**
     * @brief Retrieves a node from the graph by its unique identifier.
     *
     * @param id The unique identifier of the node to retrieve.
     * @return A pointer to the node with the specified ID, or nullptr if no
     * such node exists.
     */
    NodeBase *GetNode(uint32_t id) const;

    /**
     * @brief Retrieves a node from the graph by its unique identifier with type
     * safety.
     *
     * @tparam T The type of the node to retrieve. This should derive from
     * NodeBase.
     * @param id The unique identifier of the node to retrieve.
     * @return A pointer to the node of type T with the specified ID, or nullptr
     * if no such node exists.
     */
    template <typename T>
    T *GetNode(uint32_t id) const;

    /**
     * @brief Links two nodes by connecting an output pin to an input pin.
     *
     * @param from The source node.
     * @param out_pin The output pin index on the source node.
     * @param to The destination node.
     * @param in_pin The input pin index on the destination node.
     * @return An expected containing void on success, or an error message on
     * failure.
     *
     * @note This function validates pin compatibility and availability before
     * linking.
     */
    std::expected<void, std::string> Link(NodeBase *from, uint8_t out_pin,
                                          NodeBase *to, uint8_t in_pin);

    /**
     * @brief Unlinks two nodes by disconnecting an output pin from an input
     * pin.
     *
     * @param from The source node.
     * @param out_pin The output pin index on the source node.
     * @param to The destination node.
     * @param in_pin The input pin index on the destination node.
     * @return An expected containing void on success, or an error message on
     * failure.
     */
    std::expected<void, std::string> Unlink(NodeBase *from, uint8_t out_pin,
                                            NodeBase *to, uint8_t in_pin);

   private:
    /**
     * @brief Factory method to create a node based on its kind.
     *
     * @param id The unique identifier for the new node.
     * @param kind The kind of node to create.
     * @return A unique pointer to the newly created node.
     */
    std::unique_ptr<NodeBase> CreateNode(uint32_t id, NodeBase::NodeKind kind);

    uint32_t next_id_ = 0;
    std::vector<std::unique_ptr<NodeBase>> nodes_;
};

}  // namespace core

#include "graph.tcc"
