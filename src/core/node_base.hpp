#pragma once
#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <raylib.h>
#include <tuple>

namespace core {

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
        NodeBase *node = nullptr;  ///< Pointer to the connected node
        uint8_t pin = 0;           ///< Pin number on the connected node
        PinDataType type =
            PinDataType::kUndefined;  ///< Data type of the connection

        Connection() = default;

        /**
         * @brief Constructs a connection with specified node and pin.
         * @param node Pointer to the connected node.
         * @param pin Pin number on the connected node.
         * @param type Data type going trough the connection
         */
        Connection(NodeBase *node, uint8_t pin, PinDataType type);

        bool IsConnected() const;
    };

    virtual ~NodeBase() = default;

    uint32_t id() const;

    NodeKind kind() const;

    std::pair<float, float> GetPosition() const;

    bool IsMouseOver() const;

    std::tuple<unsigned char, unsigned char, unsigned char> GetInitialColor() const;

    /**
     * @brief Retrieves connection information for a given input pin.
     * @param input_pin The index of the input pin.
     * @return Pointer to the connection information, or nullptr if not
     * connected.
     */
    const Connection *parent(uint8_t input_pin) const;

    /**
     * @brief Retrieves all connections for a given output pin.
     * @param output_pin The index of the output pin.
     * @return Reference to a vector of connection pointers.
     */
    const std::vector<Connection> &childrens(uint8_t output_pin) const;

    virtual uint8_t GetInputPinCount() const = 0;
    virtual uint8_t GetOutputPinCount() const = 0;

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
        uint8_t target_in_pin) const = 0;

    virtual std::string GetDisplayName() const = 0;

    /**
     * @brief Gets the category of this node.
     *
     * Used for organizing nodes in the UI, such as grouping by library
     * (e.g., "STL", "SDL2", "Math", etc.).
     *
     * @return The category name for this node.
     */
    virtual std::string GetCategory() const = 0;


    /**
     *@brief Draws the node in the editor. 
     */
    virtual void Draw();

    /**
     * @brief Select the node
     */
    void ClickNode();

    /**
     * @brief Updates the node position
     */
    void MoveNode();

    /**
     * @brief Set the node color
     */
    void SetColor(unsigned char r, unsigned char g, unsigned char b);

    /**
     * @brief prepare the node for dragging
     */
    void PrepareDrag();

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
    void RemoveChild(uint8_t output_pin, const NodeBase *node, uint8_t input_pin);

   protected:
    /**
     * @brief Protected constructor to prevent direct instantiation.
     *
     * The NodeBase constructor can only be called by class inheriting from NodeBase
     *
     * @param id Unique identifier for this node.
     * @param kind The type/kind of this node.
     */
    NodeBase(uint32_t id, NodeKind kind, std::pair<float, float> position = {0.0f, 0.0f});

    const uint32_t id_;
    const NodeKind kind_;
    std::pair<float, float> position_ = {0.0f, 0.0f};  ///< Position in the editor
    std::pair<float, float> drag_offset_ = {0.0f, 0.0f};  ///< Offset used for dragging the node
    std::pair<float, float> initial_position_cursor_ = {0.0f, 0.0f};  ///< Initial position for resetting
    std::pair<float, float> initial_position_ = {0.0f, 0.0f};  ///< Initial position for resetting
    bool follow_mouse_ = false;
    std::tuple <unsigned char, unsigned char, unsigned char> color_ = {130, 130, 130};
    std::tuple <unsigned char, unsigned char, unsigned char> initial_color_ = {130, 130, 130};

    std::vector<Connection> parents_;
    std::vector<std::vector<Connection>> childrens_;
};

}  // namespace core
