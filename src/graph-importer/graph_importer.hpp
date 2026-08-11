#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "graph.hpp"
#include "node_base.hpp"
#include "nodes/operator_node.hpp"

namespace graph_importer {

class GraphImporter {
   public:
    GraphImporter() = default;

    /**
     * @brief Parses C++ source text and builds an equivalent node graph
     * @param source_code .cpp file in a text format
     * @return A graph or an error message
     */
    std::expected<core::Graph, std::string> ImportCpp(
        const std::string &source_code) const;

    /**
     * @brief Reads a .cpp file from disk and imports it into a graph
     * @param file_path Path to the source file to import
     */
    std::expected<core::Graph, std::string> ImportFromFile(
        const std::filesystem::path &file_path) const;

   private:
    enum class TokenType {
        kIdentifier,
        kIntLiteral,
        kFloatLiteral,
        kStringLiteral,
        kBoolLiteral,
        kKeyword,
        kPunct,
        kOperator,
        kEnd
    };

    struct Token {
        TokenType type = TokenType::kEnd;
        std::string text;
        int line = 1;
    };

    static std::expected<std::vector<Token>, std::string> Tokenize(
        const std::string &source);

    struct ParseState {
        const std::vector<Token> *tokens = nullptr;
        size_t pos = 0;
        core::Graph *graph = nullptr;
        // Maps a variable name to the node whose output pin currently
        // represents that variable's value (its VariableNode).
        std::unordered_map<std::string, core::NodeBase *> symbol_table;
        // For the display
        float x_cursor = 0.0f;
        float y_cursor = 0.0f;
    };

    const Token &Peek(const ParseState &state) const;
    const Token &Advance(ParseState &state) const;
    bool Check(const ParseState &state, const std::string &text) const;
    bool CheckType(const ParseState &state, TokenType type) const;
    std::expected<Token, std::string> Expect(ParseState &state,
                                             const std::string &text) const;
    utils::WrappedVector2 NextPosition(ParseState &state) const;
    std::string ErrorAt(const ParseState &state,
                        const std::string &message) const;

    std::expected<void, std::string> ParseProgram(ParseState &state) const;

    // Parses one statement. If non-null, `body_out` collects the resulting
    // root node of the statement so callers (if/while/for) can attach it as
    // part of a control block's body. `in_block` is true while collecting a
    // parent control-block's body: nested if/while/for are rejected in that
    // case, since ConditionNode/LoopNode/ForNode expose no "control" input
    // pin an enclosing block could attach through (only PrintNode does).
    std::expected<void, std::string> ParseStatement(
        ParseState &state, std::vector<core::NodeBase *> *body_out,
        bool in_block) const;

    std::expected<void, std::string> ParseBlock(
        ParseState &state, std::vector<core::NodeBase *> *body_out) const;

    std::expected<core::NodeBase *, std::string> ParseDeclaration(
        ParseState &state) const;

    std::expected<core::NodeBase *, std::string> ParsePrintStatement(
        ParseState &state) const;

    std::expected<core::NodeBase *, std::string> ParseIfStatement(
        ParseState &state) const;

    std::expected<core::NodeBase *, std::string> ParseWhileStatement(
        ParseState &state) const;

    std::expected<core::NodeBase *, std::string> ParseForStatement(
        ParseState &state) const;

    // Parses `name = expr`, `name++`, `name--`, `name += expr`, `name -= expr`
    // used for the for-loop increment clause. Returns the node producing the
    // updated value (does NOT touch the symbol table — the ForNode is
    // responsible for the actual mutation semantics at codegen time).
    std::expected<core::NodeBase *, std::string> ParseUpdateExpression(
        ParseState &state) const;

    std::expected<core::NodeBase *, std::string> ParseExpression(
        ParseState &state, int min_precedence = 0) const;
    std::expected<core::NodeBase *, std::string> ParseUnary(
        ParseState &state) const;
    std::expected<core::NodeBase *, std::string> ParsePrimary(
        ParseState &state) const;

    std::expected<core::NodeBase *, std::string> MakeLiteral(
        ParseState &state, core::NodeBase::PinDataType type,
        const std::string &text) const;

    std::expected<core::NodeBase *, std::string> MakeVariable(
        ParseState &state, core::NodeBase::PinDataType type,
        const std::string &name, core::NodeBase *initializer) const;

    // Binary operator (A/B pins 0/1, Result pin 0).
    std::expected<core::NodeBase *, std::string> MakeOperator(
        ParseState &state, const std::string &op, core::NodeBase *lhs,
        core::NodeBase *rhs) const;

    // Unary operator (Input pin 0, Result pin 0). Only kLogicalNot and
    // kBitwiseNot are true unary OperatorTypes today.
    std::expected<core::NodeBase *, std::string> MakeUnaryOperator(
        ParseState &state, core::OperatorNode::OperatorType type,
        core::NodeBase *operand) const;

    // Fans the control node's named void output pin out to the "control"
    // input of every PrintNode in body_nodes, in encounter order (relying on
    // connection insertion order for emission order - see the note in the
    // .cpp). Declarations (VariableNode) have no control pin and are left
    // for the topological/data-dependency pass to place; this is the part
    // most in need of confirmation against CodegenContext.
    std::expected<void, std::string> AttachBody(
        core::NodeBase *control_node, const std::string &body_pin_name,
        const std::vector<core::NodeBase *> &body_nodes,
        core::Graph &graph) const;

    static std::expected<uint8_t, std::string> FindInputPin(
        const core::NodeBase &node, const std::string &name);
    static std::expected<uint8_t, std::string> FindOutputPin(
        const core::NodeBase &node, const std::string &name);
    static std::expected<core::OperatorNode::OperatorType, std::string>
    BinaryOpToOperatorType(const std::string &op);

    static bool IsTypeKeyword(const std::string &text);
    static core::NodeBase::PinDataType KeywordToPinType(
        const std::string &text);
    static int BinaryPrecedence(const std::string &op);
    static bool IsRightAssociative(const std::string &op);
};

}  // namespace graph_importer