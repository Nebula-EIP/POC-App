/**
 * This version of the Nebula Node Graph V0.
 * Purposely built to handle basic C without includes nor macros.
 */

#pragma once
#include <chrono>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "node_base.hpp"

namespace core {

class FunctionNode;

class Graph {
   public:
    Graph();
    ~Graph() = default;

    // Graph is move-only due to unique_ptr ownership
    Graph(const Graph &) = delete;
    Graph &operator=(const Graph &) = delete;
    Graph(Graph &&) = default;
    Graph &operator=(Graph &&) = default;

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

    /**
     * @brief Gets the project name.
     * @return The name of the project.
     */
    const std::string &GetProjectName() const;

    /**
     * @brief Gets the project version.
     * @return The version string of the project.
     */
    const std::string &GetVersion() const;

    /**
     * @brief Gets the project author.
     * @return The author of the project.
     */
    const std::string &GetAuthor() const;

    /**
     * @brief Gets the project creation timestamp.
     * @return The time point when the project was created.
     */
    std::chrono::system_clock::time_point GetCreatedAt() const;

    /**
     * @brief Gets the project last modification timestamp.
     * @return The time point when the project was last modified.
     */
    std::chrono::system_clock::time_point GetModifiedAt() const;

    /**
     * @brief Sets the project name.
     * @param name The new project name.
     */
    void SetProjectName(const std::string &name);

    /**
     * @brief Sets the project author.
     * @param author The new author name.
     */
    void SetAuthor(const std::string &author);

    /**
     * @brief Updates the project modification timestamp to the current time.
     */
    void UpdateModifiedTime();

    /**
     * @brief Serializes the entire graph to JSON format.
     *
     * Converts the graph and all its data into a JSON object following the
     * .nebula file format specification. Includes metadata, nodes, and
     * connections.
     *
     * @return JSON object containing the serialized graph data.
     */
    nlohmann::json Serialize() const;

    /**
     * @brief Deserializes a complete graph from JSON.
     *
     * Factory method that reconstructs a Graph object and all its nodes and
     * connections from JSON data in the .nebula format.
     *
     * @param json The JSON object containing the complete graph data.
     * @return An expected containing the deserialized Graph,
     *         or an error message if deserialization fails.
     */
    static std::expected<Graph, std::string> Deserialize(
        const nlohmann::json &json);

    /**
     * @brief Saves the graph to a .nebula file.
     *
     * Serializes the graph and writes it to the specified file path as JSON.
     * The file will be created or overwritten. Updates the modified timestamp
     * before saving.
     *
     * @param path The file path where the graph should be saved.
     * @return An expected containing void on success, or an error message if
     *         the save fails (e.g., permission denied, disk full, invalid
     * path).
     */
    std::expected<void, std::string> SaveToFile(
        const std::filesystem::path &path);

    /**
     * @brief Loads a graph from a .nebula file.
     *
     * Reads and deserializes a .nebula file from the specified path,
     * reconstructing the complete graph with all nodes and connections.
     *
     * @param path The file path to load from.
     * @return An expected containing the loaded Graph on success, or an error
     *         message if loading fails (file not found, corrupted, invalid
     * format).
     */
    static std::expected<Graph, std::string> LoadFromFile(
        const std::filesystem::path &path);

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

    // Project metadata
    std::string project_name_;
    std::string version_;
    std::string author_;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point modified_at_;
};

}  // namespace core

#include "graph.tcc"
