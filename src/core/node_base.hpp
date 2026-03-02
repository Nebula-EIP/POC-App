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

    NodeBase(uint32_t id, NodeKind kind);
    virtual ~NodeBase() = default;

    uint32_t id() const;

    NodeKind kind() const;

    bool IsInput(uint8_t pin) const;
    bool isOutput(uint8_t pin) const;

    NodeBase *parent(uint8_t input_pin) const;
    const std::vector<NodeBase *> &childrens(uint8_t output_pin) const;

    virtual uint8_t GetInputPinCount() const = 0;
    virtual uint8_t GetOutputPinCount() const = 0;

    virtual PinDataType GetInputPinType(uint8_t pin) const = 0;
    virtual PinDataType GetOutputPinType(uint8_t pin) const = 0;

    virtual bool CanConnectTo(uint8_t out_pin, const NodeBase* target, uint8_t in_pin) const;

    virtual std::string GetInputPinName(uint8_t pin) const = 0;
    virtual std::string GetOutputPinName(uint8_t pin) const = 0;

    virtual std::string GetDisplayName() const = 0;
    virtual std::string GetCategory() const;
 protected:
    friend class Graph;

    void SetParent(uint8_t pin, NodeBase* node);
    void AddChild(uint8_t pin, NodeBase* node);
  
    void RemoveChild(uint8_t pin, NodeBase* node);
    void ClearParent(uint8_t pin);

    const uint32_t id_;
    const NodeKind kind_;

    std::vector<NodeBase*> parents_;
    std::vector<std::vector<NodeBase*>> children_;
};

} // namespace core
