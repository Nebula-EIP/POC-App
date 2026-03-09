#pragma once
#include <chrono>
#include <cstdint>
#include <expected>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <utility>

#include "id_manager.hpp"

namespace core {

// Forward declarations
class Graph;
class LiteralNode;

/**
 * @class NodeBase
 * @brief Abstract base class for all nodes in the graph system.
 *
 * NodeBase provides the common interface and functionality for all node types.
 * Nodes can only be created and managed by the Graph class through the factory
 * pattern.
 */
class NodeBase {
   public:
    /**
     * @enum NodeKind
     * @brief Defines the different types of nodes available in the graph.
     */
    enum class NodeKind {
        kUndefined,       ///< Error kind, represents an invalid node type
        kLiteral,         ///< Literal value node (constant)
        kVariable,        ///< Variable reference node
        kFunction,        ///< Function call node
        kFunctionInput,   ///< Function input parameter node
        kFunctionOutput,  ///< Function output/return node
        kOperator,        ///< Operator node (arithmetic, logical, etc.)
        kCondition,       ///< Conditional branching node (if/else)
        kLoop             ///< Loop control node (for/while)
    };

    /**
     * @enum PinDataType
     * @brief Represents the data type that a pin supports.
     */
    enum class PinDataType {
        kUndefined =
            0,    ///< Error type, represents an invalid or uninitialized type
        kInt,     ///< Integer type
        kFloat,   ///< Floating-point type
        kBool,    ///< Boolean type
        kString,  ///< String type
        kVoid     ///< Void type (for execution flow pins)
    };

    /**
     * @struct Connection
     * @brief Represents a connection between two nodes' pins.
     *
     * Stores information about the connected node, the specific pin number,
     * and the data type of the connection.
     */
    struct Connection {
        NodeBase *node = nullptr;     ///< Pointer to the connected node
        uint8_t out_pin = 0;          ///< Output pin of the connection
        uint8_t in_pin = 0;           ///< Input pin of the connection
        PinDataType type =
            PinDataType::kUndefined;  ///< Data type of the connection

        Connection() noexcept = default;

        /**
         * @brief Constructs a connection with specified node and pin.
         * @param node Pointer to the connected node.
         * @param out_pin Pin number on the node who owns the connection
         * @param in_pin Pin number on the connected node.
         * @param type Data type going trough the connection
         */
        Connection(NodeBase *node, uint8_t out_pin, uint8_t in_pin, PinDataType type) noexcept;

        bool IsConnected() const noexcept;
    };

    virtual ~NodeBase() noexcept;

    uint32_t id() const noexcept;

    NodeKind kind() const noexcept;

    /**
     * @brief Retrieves connection information for a given input pin.
     * @param input_pin The index of the input pin.
     * 
     * @throws `InvalidPinIndexException` if input_pin >= GetInputPinCount()
     * @throws `PinNotConnectedException` if the pin exists but is not connected 
     * 
     * @return Connection struct representing the pin's connection
     */
    Connection parent(uint8_t input_pin) const;

    /**
     * @brief Retrieves all connections for a given output pin.
     * @param output_pin The index of the output pin.
     * @return Pointer to a vector of connection pointers.
     */
    const std::vector<Connection> *childrens(uint8_t output_pin) const;

    virtual uint8_t GetInputPinCount() const noexcept = 0;
    virtual uint8_t GetOutputPinCount() const noexcept = 0;

    virtual PinDataType GetInputPinType(uint8_t pin) const = 0;
    virtual PinDataType GetOutputPinType(uint8_t pin) const = 0;

    virtual std::string GetInputPinName(uint8_t pin) const = 0;
    virtual std::string GetOutputPinName(uint8_t pin) const = 0;

    /**
     * @brief Validates if this node's output pin can connect to a target node's
     * input pin.
     *
     * This method should be called from the source node to check if a
     * connection is valid. To check if a node can receive a connection, call
     * this method from the source node.
     *
     * @param out_pin The output pin index on this node.
     * @param target Pointer to the target node.
     * @param target_in_pin The input pin index on the target node.
     * @return An expected containing void on success, or an error message on
     * failure.
     */
    virtual std::expected<void, std::string> CanConnectTo(
        uint8_t out_pin, const NodeBase *target,
        uint8_t target_in_pin) const noexcept = 0;

    virtual std::string GetDisplayName() const noexcept = 0;

    /**
     * @brief Gets the category of this node.
     *
     * Used for organizing nodes in the UI, such as grouping by library
     * (e.g., "STL", "SDL2", "Math", etc.).
     *
     * @return The category name for this node.
     */
    virtual std::string GetCategory() const noexcept = 0;

    /**
     * @brief Serializes the node to JSON format.
     *
     * Converts this node's data into a JSON object following the .nebula
     * format specification. The JSON must include the node's id and kind,
     * plus any node-specific properties.
     *
     * @return JSON object containing the serialized node data.
     */
    virtual nlohmann::json Serialize() const = 0;

    /**
     * @brief Deserializes this node's data from JSON.
     *
     * Called by the factory after the node is constructed to initialize its
     * fields from the JSON data. Each derived class implements this to load
     * its specific properties.
     *
     * @param json The JSON object containing the node's data.
     * @return An expected containing void on success, or an error message on
     *         failure.
     */
    virtual std::expected<void, std::string> Deserialize(
        const nlohmann::json &json) = 0;

    /**
     * @brief Factory method to deserialize a node from JSON.
     *
     * Creates the appropriate node type based on the "kind" field in JSON,
     * then calls its virtual Deserialize method to initialize it. The JSON
     * must contain "id" and "kind" fields at a minimum.
     *
     * @param json The JSON object containing the node data.
     * @param graph Pointer to the owning Graph (friend class).
     * @return An expected containing a unique_ptr to the deserialized node,
     *         or an error message if deserialization fails.
     */
    static std::expected<std::unique_ptr<NodeBase>, std::string>
    DeserializeFactory(const nlohmann::json &json, Graph *graph);

   protected:
    friend class Graph;  ///< Graph class manages the lifetime of nodes

    /**
     * @brief Sets an input pin connection.
     *
     * If the input pin already has a connection, it will be overridden.
     *
     * @param input_pin The input pin index on this node.
     * @param node Pointer to the parent node.
     * @param parent_pin The output pin index on the parent node.
     */
    void SetParent(uint8_t input_pin, NodeBase *node, uint8_t parent_pin);

    /**
     * @brief Adds a child connection from an output pin.
     *
     * Does not override previous connections - multiple children can be
     * connected to the same output pin.
     *
     * @param output_pin The output pin index on this node.
     * @param node Pointer to the child node.
     * @param child_pin The input pin index on the child node.
     */
    void AddChild(uint8_t output_pin, NodeBase *node, uint8_t child_pin);

    /**
     * @brief Resets an input pin connection.
     *
     * Disconnects the specified input pin and cleans up the connection data.
     *
     * @param pin The input pin index to clear.
     */
    void ClearParent(uint8_t pin);

    /**
     * @brief Removes a specific child connection from an output pin.
     *
     * @param output_pin The output pin index on this node.
     * @param node Pointer to the child node to disconnect.
     * @param input_pin The input pin index on the child node.
     */
    void RemoveChild(uint8_t output_pin, const NodeBase *node,
                     uint8_t input_pin);

    /**
     * @brief Initializes the connection vectors based on pin counts.
     *
     * Must be called after construction to properly size the parents_
     * and childrens_ vectors and create Connection objects.
     */
    void InitializeConnections();

   protected:
    /**
     * @brief Protected constructor to prevent direct instantiation.
     *
     * The NodeBase constructor can only be called by class inheriting from
     * NodeBase
     *
     * @param id Unique identifier for this node.
     * @param kind The type/kind of this node.
     */
    NodeBase(uint32_t id, NodeKind kind) noexcept;

    const uint32_t id_;
    const NodeKind kind_;

    utils::IdManager<uint8_t> in_pin_id_manager_;
    std::vector<Connection> parents_;
    utils::IdManager<uint8_t> out_pin_id_manager_;
    std::vector<std::pair<uint8_t, std::vector<Connection>>> childrens_;
};

// Helper functions for enum to/from string conversion

std::string NodeKindToString(NodeBase::NodeKind kind);
NodeBase::NodeKind StringToNodeKind(const std::string &str);

std::string PinDataTypeToString(NodeBase::PinDataType type);
NodeBase::PinDataType StringToPinDataType(const std::string &str);

}  // namespace core
