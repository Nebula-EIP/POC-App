/**
 * @file graph_importer.hpp
 * @brief Converts supported C++ source code into a graph of nodes.
 *
 * This file contains the declaration of the GraphImporter class, which is
 * responsible for parsing a supported subset of C++ source code and converting
 * it into an equivalent core::Graph representation.
 *
 * The importer supports variable declarations, literals, operators,
 * std::cout statements, if/else statements, while loops, and for loops.
 * Unsupported C++ constructs are reported as parse errors rather than being
 * silently ignored or guessed.
 *
 * The class provides methods to import C++ source code directly from a string
 * or from a .cpp file.
 *
 * @author Created by NathanBezard
 * @date Created on 09-08-2026
 *
 * @author Last modified by NathanBezard
 * @date Last modified on 11-08-2026
 */

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

/**
 * @brief Converts supported C++ source code into a core::Graph.
 *
 * GraphImporter parses C++ source code according to the subset supported by
 * the graph code-generation system and reconstructs the corresponding node
 * graph.
 *
 * The importer does not attempt to interpret unsupported C++ constructs.
 * Instead, it reports the first parsing error encountered.
 */
class GraphImporter {
   public:
    /**
     * @brief Default constructor.
     *
     * Creates a GraphImporter instance without any additional configuration.
     */
    GraphImporter() = default;

    /**
     * @brief Parses C++ source code and builds an equivalent node graph.
     *
     * The source code is tokenized, parsed according to the supported grammar,
     * and converted into a core::Graph.
     *
     * @param source_code C++ source code to parse.
     *
     * @return A core::Graph on success, or an error message describing the
     * first parsing failure.
     */
    std::expected<core::Graph, std::string> ImportCpp(
        const std::string &source_code) const;

    /**
     * @brief Reads a C++ source file and imports it into a graph.
     *
     * The file is read from disk and its contents are passed to the C++ source
     * parser.
     *
     * @param file_path Path to the C++ source file to import.
     *
     * @return A core::Graph on success, or an error message if the file cannot
     * be read or the source code cannot be parsed.
     */
    std::expected<core::Graph, std::string> ImportFromFile(
        const std::filesystem::path &file_path) const;

   private:
    /**
     * @brief Represents the different types of tokens produced by the lexer.
     *
     * Tokens are used by the parser to identify identifiers, literals,
     * keywords, punctuation, and operators in the source code.
     */
    enum class TokenType {
        /** @brief Identifier such as a variable name. */
        kIdentifier,

        /** @brief Integer literal such as 42. */
        kIntLiteral,

        /** @brief Floating-point literal such as 3.14. */
        kFloatLiteral,

        /** @brief String literal such as "Hello". */
        kStringLiteral,

        /** @brief Boolean literal such as true or false. */
        kBoolLiteral,

        /** @brief C++ keyword such as int, if, while, or for. */
        kKeyword,

        /** @brief Punctuation token such as parentheses or braces. */
        kPunct,

        /** @brief Operator token such as +, ==, or +=. */
        kOperator,

        /** @brief Marks the end of the token stream. */
        kEnd
    };

    /**
     * @brief Represents a single token produced by the lexer.
     *
     * Each token stores its type, its textual representation, and the source
     * code line on which it was found.
     */
    struct Token {
        /** @brief Type of the token. */
        TokenType type = TokenType::kEnd;

        /** @brief Text represented by the token. */
        std::string text;

        /** @brief Source code line containing the token. */
        int line = 1;
    };

    /**
     * @brief Converts C++ source code into a sequence of tokens.
     *
     * The lexer recognizes the subset of C++ syntax supported by the importer
     * and records the source line of every token for error reporting.
     *
     * @param source Source code to tokenize.
     *
     * @return A vector of tokens on success, or an error message if the source
     * contains an invalid token.
     */
    static std::expected<std::vector<Token>, std::string> Tokenize(
        const std::string &source);

    /**
     * @brief Stores the mutable state of the parser during parsing.
     *
     * ParseState contains the current position in the token stream, the graph
     * being constructed, the symbol table used to resolve variables, and the
     * current display position used when adding nodes to the graph.
     */
    struct ParseState {
        /** @brief Token sequence currently being parsed. */
        const std::vector<Token> *tokens = nullptr;

        /** @brief Index of the next token to be processed. */
        size_t pos = 0;

        /** @brief Graph currently being constructed. */
        core::Graph *graph = nullptr;

        /**
         * @brief Maps variable names to the nodes representing their values.
         *
         * The mapped node represents the current value associated with the
         * corresponding variable.
         */
        std::unordered_map<std::string, core::NodeBase *> symbol_table;

        /** @brief Horizontal position used when placing newly created nodes. */
        float x_cursor = 0.0f;

        /** @brief Vertical position used when placing newly created nodes. */
        float y_cursor = 0.0f;
    };

    /**
     * @brief Returns the current token without advancing the parser.
     *
     * @param state Current parser state.
     *
     * @return Reference to the current token.
     */
    const Token &Peek(const ParseState &state) const;

    /**
     * @brief Returns the current token and advances the parser.
     *
     * @param state Current parser state.
     *
     * @return Reference to the token that was consumed.
     */
    const Token &Advance(ParseState &state) const;

    /**
     * @brief Checks whether the current token has the specified text.
     *
     * @param state Current parser state.
     * @param text Expected token text.
     *
     * @return true if the current token matches the specified text, false
     * otherwise.
     */
    bool Check(const ParseState &state, const std::string &text) const;

    /**
     * @brief Checks whether the current token has the specified type.
     *
     * @param state Current parser state.
     * @param type Expected token type.
     *
     * @return true if the current token has the specified type, false
     * otherwise.
     */
    bool CheckType(const ParseState &state, TokenType type) const;

    /**
     * @brief Consumes a token with the specified text.
     *
     * If the current token does not match the expected text, an error
     * containing the current source location is returned.
     *
     * @param state Current parser state.
     * @param text Expected token text.
     *
     * @return The consumed token on success, or a parse error otherwise.
     */
    std::expected<Token, std::string> Expect(ParseState &state,
                                             const std::string &text) const;

    /**
     * @brief Computes the next display position for a newly created node.
     *
     * The position is used only for the visual layout of nodes in the
     * resulting graph.
     *
     * @param state Current parser state.
     *
     * @return Position at which the next node should be placed.
     */
    utils::WrappedVector2 NextPosition(ParseState &state) const;

    /**
     * @brief Creates a formatted parser error at the current token.
     *
     * The generated error contains the current source line and the supplied
     * error message.
     *
     * @param state Current parser state.
     * @param message Description of the parsing error.
     *
     * @return Formatted error message.
     */
    std::string ErrorAt(const ParseState &state,
                        const std::string &message) const;

    /**
     * @brief Parses the complete C++ program.
     *
     * Parses statements until the end of the token stream is reached and
     * populates the graph stored in the parser state.
     *
     * @param state Current parser state.
     *
     * @return Success if the program is parsed successfully, or a parse error.
     */
    std::expected<void, std::string> ParseProgram(ParseState &state) const;

    /**
     * @brief Parses a single statement.
     *
     * The statement may be a declaration, print statement, conditional,
     * while loop, or for loop.
     *
     * When parsing a control block, the resulting statement nodes can be
     * collected in body_out so that the parent control node can attach them
     * to its body.
     *
     * @param state Current parser state.
     * @param body_out Optional list receiving nodes belonging to a control
     * block.
     * @param in_block Indicates whether the statement is being parsed inside
     * a control block.
     *
     * @return Success if the statement is parsed successfully, or a parse
     * error.
     */
    std::expected<void, std::string> ParseStatement(
        ParseState &state, std::vector<core::NodeBase *> *body_out,
        bool in_block) const;

    /**
     * @brief Parses a block of statements enclosed by braces.
     *
     * Parses statements between an opening and closing brace and stores the
     * resulting nodes in body_out.
     *
     * @param state Current parser state.
     * @param body_out List receiving the nodes parsed from the block.
     *
     * @return Success if the block is parsed successfully, or a parse error.
     */
    std::expected<void, std::string> ParseBlock(
        ParseState &state, std::vector<core::NodeBase *> *body_out) const;

    /**
     * @brief Parses a variable declaration.
     *
     * Parses declarations such as:
     *
     * @code
     * int value = 42;
     * float result = 3.14f;
     * bool enabled = true;
     * @endcode
     *
     * @param state Current parser state.
     *
     * @return The created variable node, or a parse error.
     */
    std::expected<core::NodeBase *, std::string> ParseDeclaration(
        ParseState &state) const;

    /**
     * @brief Parses a std::cout print statement.
     *
     * Parses supported output expressions and creates the corresponding
     * print node and graph connections.
     *
     * @param state Current parser state.
     *
     * @return The created print node, or a parse error.
     */
    std::expected<core::NodeBase *, std::string> ParsePrintStatement(
        ParseState &state) const;

    /**
     * @brief Parses an if/else statement.
     *
     * Parses the condition and associated statement blocks and creates the
     * corresponding condition node.
     *
     * @param state Current parser state.
     *
     * @return The created condition node, or a parse error.
     */
    std::expected<core::NodeBase *, std::string> ParseIfStatement(
        ParseState &state) const;

    /**
     * @brief Parses a while loop.
     *
     * Parses the loop condition and its body and creates the corresponding
     * loop node.
     *
     * @param state Current parser state.
     *
     * @return The created loop node, or a parse error.
     */
    std::expected<core::NodeBase *, std::string> ParseWhileStatement(
        ParseState &state) const;

    /**
     * @brief Parses a for loop.
     *
     * Parses the initialization, condition, increment expression, and loop
     * body and creates the corresponding for node.
     *
     * @param state Current parser state.
     *
     * @return The created for node, or a parse error.
     */
    std::expected<core::NodeBase *, std::string> ParseForStatement(
        ParseState &state) const;

    /**
     * @brief Parses a variable update expression used by a for loop.
     *
     * Supported update expressions include:
     *
     * @code
     * name = expression
     * name++
     * name--
     * name += expression
     * name -= expression
     * @endcode
     *
     * The symbol table is not modified by this function. The resulting update
     * node is used by the ForNode to perform the mutation semantics.
     *
     * @param state Current parser state.
     *
     * @return The node producing the updated value, or a parse error.
     */
    std::expected<core::NodeBase *, std::string> ParseUpdateExpression(
        ParseState &state) const;

    /**
     * @brief Parses an expression using operator precedence.
     *
     * Expressions are parsed using precedence climbing so that operators with
     * higher precedence are evaluated before operators with lower precedence.
     *
     * @param state Current parser state.
     * @param min_precedence Minimum precedence accepted by the current parsing
     * level.
     *
     * @return The root node of the parsed expression, or a parse error.
     */
    std::expected<core::NodeBase *, std::string> ParseExpression(
        ParseState &state, int min_precedence = 0) const;

    /**
     * @brief Parses a unary expression.
     *
     * Handles unary operators and delegates operand parsing to the primary
     * expression parser when appropriate.
     *
     * @param state Current parser state.
     *
     * @return The root node of the parsed unary expression, or a parse error.
     */
    std::expected<core::NodeBase *, std::string> ParseUnary(
        ParseState &state) const;

    /**
     * @brief Parses a primary expression.
     *
     * Primary expressions include literals, identifiers, and parenthesized
     * expressions supported by the importer.
     *
     * @param state Current parser state.
     *
     * @return The root node of the parsed expression, or a parse error.
     */
    std::expected<core::NodeBase *, std::string> ParsePrimary(
        ParseState &state) const;

    /**
     * @brief Creates a literal node from parsed source text.
     *
     * Converts the supplied literal text to the requested graph pin data type
     * and adds the corresponding literal node to the graph.
     *
     * @param state Current parser state.
     * @param type Data type represented by the literal.
     * @param text Textual representation of the literal.
     *
     * @return The created literal node, or an error if the literal cannot be
     * converted.
     */
    std::expected<core::NodeBase *, std::string> MakeLiteral(
        ParseState &state, core::NodeBase::PinDataType type,
        const std::string &text) const;

    /**
     * @brief Creates a variable node and registers it in the symbol table.
     *
     * The initializer is connected to the variable when provided.
     *
     * @param state Current parser state.
     * @param type Data type of the variable.
     * @param name Name of the variable.
     * @param initializer Node providing the initial variable value.
     *
     * @return The created variable node, or an error if the variable cannot be
     * created.
     */
    std::expected<core::NodeBase *, std::string> MakeVariable(
        ParseState &state, core::NodeBase::PinDataType type,
        const std::string &name, core::NodeBase *initializer) const;

    /**
     * @brief Creates a binary operator node.
     *
     * The left and right operands are connected to the operator's input pins,
     * and the resulting value is exposed through the operator's result pin.
     *
     * @param state Current parser state.
     * @param op Operator represented by the node.
     * @param lhs Left-hand operand.
     * @param rhs Right-hand operand.
     *
     * @return The created operator node, or an error if the operator is not
     * supported.
     */
    std::expected<core::NodeBase *, std::string> MakeOperator(
        ParseState &state, const std::string &op, core::NodeBase *lhs,
        core::NodeBase *rhs) const;

    /**
     * @brief Creates a unary operator node.
     *
     * The operand is connected to the operator input pin and the resulting
     * value is exposed through the result pin.
     *
     * @param state Current parser state.
     * @param type Type of unary operator to create.
     * @param operand Operand consumed by the operator.
     *
     * @return The created unary operator node, or an error if the operator is
     * not supported.
     */
    std::expected<core::NodeBase *, std::string> MakeUnaryOperator(
        ParseState &state, core::OperatorNode::OperatorType type,
        core::NodeBase *operand) const;

    /**
     * @brief Attaches statement nodes to a control node's body output.
     *
     * The control output is connected to the control input of supported nodes
     * in the order in which they appear in body_nodes.
     *
     * Variable declarations do not have a control input and are therefore
     * left to the graph's data-dependency ordering.
     *
     * @param control_node Control node providing the body output.
     * @param body_pin_name Name of the control output pin.
     * @param body_nodes Nodes belonging to the control block.
     * @param graph Graph containing the nodes and connections.
     *
     * @return Success if all applicable body nodes are attached, or an error.
     */
    std::expected<void, std::string> AttachBody(
        core::NodeBase *control_node, const std::string &body_pin_name,
        const std::vector<core::NodeBase *> &body_nodes,
        core::Graph &graph) const;

    /**
     * @brief Finds an input pin by its name.
     *
     * @param node Node whose input pins are searched.
     * @param name Name of the input pin.
     *
     * @return The input pin index, or an error if the pin does not exist.
     */
    static std::expected<uint8_t, std::string> FindInputPin(
        const core::NodeBase &node, const std::string &name);

    /**
     * @brief Finds an output pin by its name.
     *
     * @param node Node whose output pins are searched.
     * @param name Name of the output pin.
     *
     * @return The output pin index, or an error if the pin does not exist.
     */
    static std::expected<uint8_t, std::string> FindOutputPin(
        const core::NodeBase &node, const std::string &name);

    /**
     * @brief Converts a C++ binary operator into its graph operator type.
     *
     * @param op C++ operator to convert.
     *
     * @return Corresponding OperatorNode type, or an error if the operator is
     * unsupported.
     */
    static std::expected<core::OperatorNode::OperatorType, std::string>
    BinaryOpToOperatorType(const std::string &op);

    /**
     * @brief Checks whether a string represents a supported C++ type keyword.
     *
     * @param text Text to check.
     *
     * @return true if the text represents a supported type, false otherwise.
     */
    static bool IsTypeKeyword(const std::string &text);

    /**
     * @brief Converts a C++ type keyword into a graph pin data type.
     *
     * @param text C++ type keyword to convert.
     *
     * @return Corresponding graph pin data type.
     */
    static core::NodeBase::PinDataType KeywordToPinType(
        const std::string &text);

    /**
     * @brief Returns the precedence of a binary operator.
     *
     * Higher values represent operators that bind more strongly.
     *
     * @param op Operator whose precedence should be determined.
     *
     * @return Operator precedence, or a value indicating that the operator is
     * not supported.
     */
    static int BinaryPrecedence(const std::string &op);

    /**
     * @brief Checks whether an operator is right-associative.
     *
     * @param op Operator to check.
     *
     * @return true if the operator is right-associative, false otherwise.
     */
    static bool IsRightAssociative(const std::string &op);
};

}  // namespace graph_importer