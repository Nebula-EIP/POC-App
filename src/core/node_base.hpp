#pragma once
#include <chrono>
#include <cstdint>
#include <expected>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>

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
     * Stores information about the connected node, the specific pin numbers,
     * the data type, and the names of both involved pins.
     */
    struct Connection {
        NodeBase *node = nullptr;  ///< Pointer to the connected node
        uint8_t out_pin = 0;       ///< Output pin of the connection
        uint8_t in_pin = 0;        ///< Input pin of the connection
        PinDataType type =
            PinDataType::kUndefined;  ///< Data type of the connection
        std::string out_pin_name;     ///< Name of the source output pin
        std::string in_pin_name;      ///< Name of the destination input pin

        Connection() noexcept = default;

        /**
         * @brief Constructs a connection with specified node, pin, type and
         * names.
         * @param node Pointer to the connected node.
         * @param out_pin Pin number on the node who owns the connection
         * @param in_pin Pin number on the connected node.
         * @param type Data type going through the connection
         * @param out_pin_name Name of the source output pin
         * @param in_pin_name Name of the destination input pin
         */
        Connection(NodeBase *node, uint8_t out_pin, uint8_t in_pin,
                   PinDataType type, std::string out_pin_name = "",
                   std::string in_pin_name = "") noexcept;

        bool IsConnected() const noexcept;
    };

    virtual ~NodeBase() noexcept;

    uint32_t id() const noexcept;

    NodeKind kind() const noexcept;

    /**
     * @brief Retrieves connection information for a given input pin.
     * @param input_pin The index of the input pin.
     *
     * @throws `InvalidPinIndexException` if the pin does not exists
     *
     * @return Connection struct representing the pin's connection
     */
    Connection parent(uint8_t input_pin) const;

    /**
     * @brief Retreives all incoming connections of the node
     *
     * @return A reference to a vector of all Connections
     */
    const std::vector<Connection> &GetAllParents() const noexcept;

    /**
     * @brief Retrieves all connections for a given output pin.
     * @param output_pin The index of the output pin.
     *
     * @throws `InvalidPinIndexException` if the pin does not exists
     *
     * @return Pointer to a vector of Connection structs.
     */
    const std::vector<Connection> *childrens(uint8_t output_pin) const;

    /**
     * @brief Retreives all outgoing connections of the node
     *
     * @return A reference to a vector of all Connections
     */
    const std::vector<Connection> &GetAllChildrens() const noexcept;

    uint8_t GetInputPinCount() const noexcept;
    uint8_t GetOutputPinCount() const noexcept;

    bool InputPinExists(uint8_t pin) const noexcept;
    bool OutputPinExists(uint8_t pin) const noexcept;

    bool IsInputPinConnected(uint8_t pin) const noexcept;
    bool IsOutputPinConnected(uint8_t pin) const noexcept;

    PinDataType GetInputPinType(uint8_t pin) const;
    PinDataType GetOutputPinType(uint8_t pin) const;

    std::string GetInputPinName(uint8_t pin) const;
    std::string GetOutputPinName(uint8_t pin) const;

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
     * @struct OutputPin
     * @brief Stores the metadata and connections for a single output pin.
     */
    struct OutputPin {
        uint8_t id = 0;    ///< Unique pin id on this node
        std::string name;  ///< Display name of this output pin
        PinDataType type = PinDataType::kUndefined;  ///< Data type of this pin
        std::vector<Connection> connections;  ///< Active outgoing connections
    };

    /**
     * @brief Sets an input pin connection.
     *
     * @param input_pin The input pin index on this node.
     * @param node Pointer to the parent node, must not be nullptr.
     * @param parent_pin The output pin index on the parent node.
     *
     * @warning
     * If the input pin already has a connection, it will be overridden.
     *
     * It is the caller responsibility to check the function's arguments before
     * calling it and if said connection is a valid one.
     */
    void SetParent(uint8_t input_pin, NodeBase *node,
                   uint8_t parent_pin) noexcept;

    /**
     * @brief Adds a child connection from an output pin.
     *
     * Does not override previous connections - multiple children can be
     * connected to the same output pin.
     *
     * @param output_pin The output pin index on this node.
     * @param node Pointer to the child node, must not be nullptr.
     * @param child_pin The input pin index on the child node.
     *
     * @warning
     * It is the caller responsibility to check the function's arguments before
     * calling it and if said connection is a valid one.
     */
    void AddChild(uint8_t output_pin, NodeBase *node,
                  uint8_t child_pin) noexcept;

    /**
     * @brief Resets an input pin connection.
     *
     * Disconnects the specified input pin and cleans up the connection data,
     * only keeps the pin metadata (type and name) in memory.
     *
     * @param pin The input pin index to clear.
     *
     * @warning It is the caller responsibility to check the function's
     * arguments before calling it.
     */
    void ClearParent(uint8_t pin) noexcept;

    /**
     * @brief Removes a specific child connection from an output pin.
     *
     * @param output_pin The output pin index on this node.
     * @param node Pointer to the child node to disconnect, must not be nullptr.
     * @param input_pin The input pin index on the child node.
     *
     * @warning It is the caller responsibility to check the function's
     * arguments before calling it.
     */
    void RemoveChild(uint8_t output_pin, const NodeBase *node,
                     uint8_t input_pin) noexcept;

    /**
     * @brief Initializes the pin vectors based on the node's pin layout.
     *
     * Must be implemented & called by class inheriting from this class in order
     * to setup the pins properly. Uses AddInputPin() / AddOutputPin() to
     * declare each pin with its name and type.
     */
    virtual void InitializeConnections() = 0;

    /**
     * @brief Adds a new input pin to this node.
     *
     * Allocates a unique pin id and appends the pin to the input pin list.
     * Should be called from `InitializeConnections()` or by the `Graph` when
     * managing dynamic pin layouts (e.g. function parameters).
     *
     * Note: These methods are `protected` to ensure only the node itself and
     * the `Graph` (friend class) can modify pin topology. External callers
     * must use `Graph` APIs to manage links/topology.
     */
    void AddInputPin(const std::string &name, PinDataType type);

    /**
     * @brief Removes an input pin by its id.
     *
     * @param pin The id of the input pin to remove.
     *
     * @throws `InvalidPinIndexException` if the pin does not exist.
     * @throws `PinStillConnectedException` if the pin is still connected.
     *         Use `Graph::Unlink()` to disconnect before removing.
     */
    void RemoveInputPin(uint8_t pin);

    /**
     * @brief Adds a new output pin to this node.
     *
     * Allocates a unique pin id and appends the pin to the output pin list.
     * Should be called from `InitializeConnections()` or by the `Graph` when
     * managing dynamic pin layouts.
     */
    void AddOutputPin(const std::string &name, PinDataType type);

    /**
     * @brief Removes an output pin by its id.
     *
     * @param pin The id of the output pin to remove.
     *
     * @throws `InvalidPinIndexException` if the pin does not exist.
     * @throws `PinStillConnectedException` if the pin still has connections.
     *         Use `Graph::Unlink()` to disconnect before removing.
     */
    void RemoveOutputPin(uint8_t pin);

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
    std::vector<Connection> parents_;  ///< Input pins (one entry per pin slot)

    utils::IdManager<uint8_t> out_pin_id_manager_;
    std::vector<OutputPin> childrens_;  ///< Output pins with their connections
};

// Helper functions for enum to/from string conversion

std::string NodeKindToString(NodeBase::NodeKind kind);
NodeBase::NodeKind StringToNodeKind(const std::string &str);

std::string PinDataTypeToString(NodeBase::PinDataType type);
NodeBase::PinDataType StringToPinDataType(const std::string &str);

}  // namespace core
