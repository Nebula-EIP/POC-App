#pragma once
#include <cstdint>
#include <memory>
#include <expected>
#include <vector>

namespace core {

class NodeBase {
 public:
    enum class NodeKind {
        kUndefined, // Error kind
        kLiteral,
        kVariable,
        kFunction,
        kFunctionInput,
        kFunctionOutput,
        kOperator,
        kCondition,
        kLoop
    };

    enum class PinDataType {
        kUndefined = 0, // Error type
        kInt,
        kFloat,
        kBool,
        kString,
        kVoid
    };

    struct Connection {
        NodeBase *node = nullptr; // Dest node
        uint8_t pin = 0;    // Dest pin

        Connection() = default;
        Connection(NodeBase *node, uint8_t pin);

        bool IsConnected() const;
    };

    NodeBase(uint32_t id, NodeKind kind);
    virtual ~NodeBase() = default;

    uint32_t id() const;

    NodeKind kind() const;

    Connection *parent(uint8_t input_pin) const;
    const std::vector<Connection *> &childrens(uint8_t output_pin) const;

    virtual uint8_t GetInputPinCount() const = 0;
    virtual uint8_t GetOutputPinCount() const = 0;

    virtual PinDataType GetInputPinType(uint8_t pin) const = 0;
    virtual PinDataType GetOutputPinType(uint8_t pin) const = 0;

    virtual bool CanConnectTo(uint8_t out_pin, const NodeBase* target, uint8_t in_pin) const = 0;

    virtual std::string GetInputPinName(uint8_t pin) const = 0;
    virtual std::string GetOutputPinName(uint8_t pin) const = 0;

    virtual std::string GetDisplayName() const = 0;
    virtual std::string GetCategory() const = 0;
 protected:
    friend class Graph;

    void SetParent(uint8_t input_pin, NodeBase* node, uint8_t parent_pin);
    void AddChild(uint8_t output_pin, NodeBase* node, uint8_t child_pin);
  
    void ClearParent(uint8_t pin);
    void RemoveChild(uint8_t output_pin, NodeBase* node, uint8_t input_pin);

    const uint32_t id_;
    const NodeKind kind_;

    std::vector<Connection*> parents_;
    std::vector<std::vector<Connection*>> childrens_;
};

} // namespace core
