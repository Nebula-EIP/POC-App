#include "graph_importer.hpp"

#include <cctype>
#include <fstream>
#include <sstream>
#include <unordered_set>

#include "nodes/condition_node.hpp"
#include "nodes/for_node.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/loop_node.hpp"
#include "nodes/operator_node.hpp"
#include "nodes/print_node.hpp"
#include "nodes/variable_node.hpp"

namespace graph_importer {

std::expected<core::Graph, std::string> GraphImporter::ImportFromFile(
    const std::filesystem::path &file_path) const {
    if (!std::filesystem::exists(file_path)) {
        return std::unexpected("File does not exist: " + file_path.string());
    }

    std::ifstream file(file_path);
    if (!file.is_open()) {
        return std::unexpected("Failed to open file for reading: " +
                               file_path.string());
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return ImportCpp(buffer.str());
}

std::expected<core::Graph, std::string> GraphImporter::ImportCpp(
    const std::string &source_code) const {
    auto tokens = Tokenize(source_code);
    if (!tokens.has_value()) {
        return std::unexpected(tokens.error());
    }

    core::Graph graph;

    ParseState state;
    state.tokens = &tokens.value();
    state.pos = 0;
    state.graph = &graph;

    auto result = ParseProgram(state);
    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    return graph;
}

std::expected<std::vector<GraphImporter::Token>, std::string>
GraphImporter::Tokenize(const std::string &source) {
    static const std::unordered_set<std::string> kKeywords = {
        "int",  "float", "double", "bool", "string", "void", "if",
        "else", "while", "for",    "true", "false",  "std"};

    std::vector<Token> tokens;
    int line = 1;
    size_t i = 0;
    const size_t kN = source.size();

    while (i < kN) {
        char c = source[i];

        if (c == '\n') {
            ++line;
            ++i;
            continue;
        }
        if (std::isspace(static_cast<unsigned char>(c))) {
            ++i;
            continue;
        }

        // Line comment
        if (c == '/' && i + 1 < kN && source[i + 1] == '/') {
            while (i < kN && source[i] != '\n') ++i;
            continue;
        }
        // Block comment
        if (c == '/' && i + 1 < kN && source[i + 1] == '*') {
            i += 2;
            while (i + 1 < kN && !(source[i] == '*' && source[i + 1] == '/')) {
                if (source[i] == '\n') ++line;
                ++i;
            }
            i += 2;
            continue;
        }

        // String literal
        if (c == '"') {
            std::string value;
            ++i;
            while (i < kN && source[i] != '"') {
                if (source[i] == '\\' && i + 1 < kN) {
                    value += source[i + 1];
                    i += 2;
                } else {
                    value += source[i];
                    ++i;
                }
            }
            if (i >= kN) {
                return std::unexpected("Unterminated string literal at line " +
                                       std::to_string(line));
            }
            ++i;
            tokens.push_back({TokenType::kStringLiteral, value, line});
            continue;
        }

        // Number literal
        if (std::isdigit(static_cast<unsigned char>(c))) {
            size_t start = i;
            bool is_float = false;
            while (i < kN &&
                   std::isdigit(static_cast<unsigned char>(source[i]))) {
                ++i;
            }
            if (i < kN && source[i] == '.') {
                is_float = true;
                ++i;
                while (i < kN &&
                       std::isdigit(static_cast<unsigned char>(source[i]))) {
                    ++i;
                }
            }
            // Trailing float suffix e.g. 3.0f
            if (i < kN && (source[i] == 'f' || source[i] == 'F')) {
                is_float = true;
                ++i;
            }
            std::string text = source.substr(start, i - start);
            tokens.push_back(
                {is_float ? TokenType::kFloatLiteral : TokenType::kIntLiteral,
                 text, line});
            continue;
        }

        // Identifier / keyword
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            size_t start = i;
            while (i < kN &&
                   (std::isalnum(static_cast<unsigned char>(source[i])) ||
                    source[i] == '_')) {
                ++i;
            }
            std::string text = source.substr(start, i - start);

            // Collapse "std::cout" / "std::endl" / "std::string" into single
            // logical tokens so the parser doesn't need to special-case "::".
            if (text == "std" && i + 1 < kN && source[i] == ':' &&
                source[i + 1] == ':') {
                size_t colon_pos = i;
                i += 2;
                size_t sub_start = i;
                while (i < kN &&
                       (std::isalnum(static_cast<unsigned char>(source[i])) ||
                        source[i] == '_')) {
                    ++i;
                }
                std::string suffix = source.substr(sub_start, i - sub_start);
                if (suffix.empty()) {
                    i = colon_pos;  // malformed, fall back
                } else {
                    text = "std::" + suffix;
                    if (text == "std::string") {
                        tokens.push_back({TokenType::kKeyword, "string", line});
                    } else {
                        tokens.push_back({TokenType::kKeyword, text, line});
                    }
                    continue;
                }
            }

            if (text == "true" || text == "false") {
                tokens.push_back({TokenType::kBoolLiteral, text, line});
            } else if (kKeywords.count(text)) {
                tokens.push_back({TokenType::kKeyword, text, line});
            } else {
                tokens.push_back({TokenType::kIdentifier, text, line});
            }
            continue;
        }

        // Multi-char operators
        auto two_char = [&](const char *op) {
            return i + 1 < kN && source[i] == op[0] && source[i + 1] == op[1];
        };
        static const char *k_two_char_ops[] = {
            "==", "!=", "<=", ">=", "&&", "||",
            "<<", ">>", "++", "--", "+=", "-="};
        bool matched_two = false;
        for (const char *op : k_two_char_ops) {
            if (two_char(op)) {
                tokens.push_back({TokenType::kOperator, op, line});
                i += 2;
                matched_two = true;
                break;
            }
        }
        if (matched_two) continue;

        static const std::string kSinglePunct = "(){};,";
        static const std::string kSingleOps = "+-*/%<>=!";

        if (kSinglePunct.find(c) != std::string::npos) {
            tokens.push_back({TokenType::kPunct, std::string(1, c), line});
            ++i;
            continue;
        }
        if (kSingleOps.find(c) != std::string::npos) {
            tokens.push_back({TokenType::kOperator, std::string(1, c), line});
            ++i;
            continue;
        }

        return std::unexpected("Unexpected character '" + std::string(1, c) +
                               "' at line " + std::to_string(line));
    }

    tokens.push_back({TokenType::kEnd, "", line});
    return tokens;
}

const GraphImporter::Token &GraphImporter::Peek(const ParseState &state) const {
    return (*state.tokens)[state.pos];
}

const GraphImporter::Token &GraphImporter::Advance(ParseState &state) const {
    const Token &t = (*state.tokens)[state.pos];
    if (state.pos + 1 < state.tokens->size()) ++state.pos;
    return t;
}

bool GraphImporter::Check(const ParseState &state,
                          const std::string &text) const {
    return Peek(state).text == text && Peek(state).type != TokenType::kEnd;
}

bool GraphImporter::CheckType(const ParseState &state, TokenType type) const {
    return Peek(state).type == type;
}

std::expected<GraphImporter::Token, std::string> GraphImporter::Expect(
    ParseState &state, const std::string &text) const {
    if (!Check(state, text)) {
        return std::unexpected(ErrorAt(
            state,
            "expected '" + text + "' but found '" + Peek(state).text + "'"));
    }
    return Advance(state);
}

std::string GraphImporter::ErrorAt(const ParseState &state,
                                   const std::string &message) const {
    return "line " + std::to_string(Peek(state).line) + ": " + message;
}

utils::WrappedVector2 GraphImporter::NextPosition(ParseState &state) const {
    utils::WrappedVector2 pos = {state.x_cursor, state.y_cursor};
    state.x_cursor += 150.0f;
    if (state.x_cursor > 900.0f) {
        state.x_cursor = 0.0f;
        state.y_cursor += 120.0f;
    }
    return pos;
}

std::expected<void, std::string> GraphImporter::ParseProgram(
    ParseState &state) const {
    while (!CheckType(state, TokenType::kEnd)) {
        auto result = ParseStatement(state, nullptr, /*in_block=*/false);
        if (!result.has_value()) return std::unexpected(result.error());
    }
    return {};
}

std::expected<void, std::string> GraphImporter::ParseStatement(
    ParseState &state, std::vector<core::NodeBase *> *body_out,
    bool in_block) const {
    const Token &tok = Peek(state);

    if (tok.type == TokenType::kKeyword && IsTypeKeyword(tok.text)) {
        auto node = ParseDeclaration(state);
        if (!node.has_value()) return std::unexpected(node.error());
        if (body_out) body_out->push_back(node.value());
        return {};
    }

    if (Check(state, "std::cout")) {
        auto node = ParsePrintStatement(state);
        if (!node.has_value()) return std::unexpected(node.error());
        if (body_out) body_out->push_back(node.value());
        return {};
    }

    if (Check(state, "if") || Check(state, "while") || Check(state, "for")) {
        if (in_block) {
            // ConditionNode/LoopNode/ForNode have no "control" input pin, so
            // there's no way for an enclosing block to sequence a nested
            // control statement the way it can a PrintNode. Rejecting this
            // explicitly rather than silently building an unlinked/ambiguous
            // graph - confirm the intended mechanism if nesting is needed.
            return std::unexpected(
                ErrorAt(state,
                        "nested if/while/for inside another block isn't "
                        "supported by the current node graph (Condition/Loop/"
                        "For nodes have no execution-order input pin)"));
        }

        if (Check(state, "if")) {
            auto node = ParseIfStatement(state);
            if (!node.has_value()) return std::unexpected(node.error());
            if (body_out) body_out->push_back(node.value());
            return {};
        }
        if (Check(state, "while")) {
            auto node = ParseWhileStatement(state);
            if (!node.has_value()) return std::unexpected(node.error());
            if (body_out) body_out->push_back(node.value());
            return {};
        }
        auto node = ParseForStatement(state);
        if (!node.has_value()) return std::unexpected(node.error());
        if (body_out) body_out->push_back(node.value());
        return {};
    }

    return std::unexpected(ErrorAt(
        state, "unsupported statement starting at '" + tok.text +
                   "' (the generator subset only supports declarations, "
                   "std::cout, if/else, while, and for)"));
}

std::expected<void, std::string> GraphImporter::ParseBlock(
    ParseState &state, std::vector<core::NodeBase *> *body_out) const {
    // Blocks are optional single-statement bodies (`if (x) foo();`) or
    // braced multi-statement bodies.
    if (Check(state, "{")) {
        Advance(state);
        while (!Check(state, "}")) {
            if (CheckType(state, TokenType::kEnd)) {
                return std::unexpected(ErrorAt(state, "unterminated block"));
            }
            auto result = ParseStatement(state, body_out, true);
            if (!result.has_value()) return std::unexpected(result.error());
        }
        Advance(state);
        return {};
    }

    return ParseStatement(state, body_out, true);
}

std::expected<core::NodeBase *, std::string> GraphImporter::ParseDeclaration(
    ParseState &state) const {
    Token type_tok = Advance(state);
    core::NodeBase::PinDataType pin_type = KeywordToPinType(type_tok.text);

    if (!CheckType(state, TokenType::kIdentifier)) {
        return std::unexpected(ErrorAt(
            state,
            "expected variable name after type '" + type_tok.text + "'"));
    }
    std::string name = Advance(state).text;

    auto eq = Expect(state, "=");
    if (!eq.has_value()) return std::unexpected(eq.error());

    auto init = ParseExpression(state);
    if (!init.has_value()) return std::unexpected(init.error());

    auto semi = Expect(state, ";");
    if (!semi.has_value()) return std::unexpected(semi.error());

    if (state.symbol_table.count(name)) {
        return std::unexpected(ErrorAt(
            state, "variable '" + name +
                       "' is already declared (reassignment isn't supported "
                       "by the generator)"));
    }

    return MakeVariable(state, pin_type, name, init.value());
}

std::expected<core::NodeBase *, std::string> GraphImporter::ParsePrintStatement(
    ParseState &state) const {
    Advance(state);

    core::NodeBase *print_node = state.graph->AddNode<core::PrintNode>(
        core::NodeBase::NodeKind::kPrint, NextPosition(state));
    if (print_node == nullptr) {
        return std::unexpected(ErrorAt(state, "failed to create PrintNode"));
    }

    // PrintNode has exactly one "value" input pin (kString-compatible), so
    // only a single non-endl operand is supported - `std::cout << a << b;`
    // has no way to concatenate `a` and `b` in the current node set (no
    // string-concat OperatorType exists).
    core::NodeBase *value_node = nullptr;

    while (Check(state, "<<")) {
        Advance(state);

        if (Check(state, "std::endl")) {
            Advance(state);
            continue;
        }

        if (value_node != nullptr) {
            return std::unexpected(
                ErrorAt(state,
                        "std::cout only supports a single printed value in "
                        "this graph (PrintNode has one 'value' pin) - split "
                        "into multiple std::cout statements"));
        }

        auto value = ParseExpression(state);
        if (!value.has_value()) return std::unexpected(value.error());
        value_node = value.value();
    }

    auto semi = Expect(state, ";");
    if (!semi.has_value()) return std::unexpected(semi.error());

    if (value_node != nullptr) {
        auto value_pin = FindInputPin(*print_node, "value");
        if (!value_pin.has_value()) return std::unexpected(value_pin.error());
        state.graph->Link(value_node, 0, print_node, value_pin.value());
    }

    // NOTE: the "control" pin (kVoid) is left unconnected here. It gets
    // wired by AttachBody() when this print is part of an if/while/for
    // body. A top-level `std::cout` has no void producer to connect from in
    // this grammar - if GraphValidator's "required connections" check
    // rejects an unconnected control pin, top-level prints need a different
    // mechanism (e.g. an implicit entry/start node) that I don't have
    // visibility into yet.
    return print_node;
}

std::expected<core::NodeBase *, std::string> GraphImporter::ParseIfStatement(
    ParseState &state) const {
    Advance(state);  // 'if'
    auto open = Expect(state, "(");
    if (!open.has_value()) return std::unexpected(open.error());

    auto condition = ParseExpression(state);
    if (!condition.has_value()) return std::unexpected(condition.error());

    auto close = Expect(state, ")");
    if (!close.has_value()) return std::unexpected(close.error());

    core::NodeBase *condition_node = state.graph->AddNode<core::ConditionNode>(
        core::NodeBase::NodeKind::kCondition, NextPosition(state));
    if (condition_node == nullptr) {
        return std::unexpected(
            ErrorAt(state, "failed to create ConditionNode"));
    }

    auto cond_pin = FindInputPin(*condition_node, "cond");
    if (!cond_pin.has_value()) return std::unexpected(cond_pin.error());
    state.graph->Link(condition.value(), 0, condition_node, cond_pin.value());

    std::vector<core::NodeBase *> then_body;
    auto then_result = ParseBlock(state, &then_body);
    if (!then_result.has_value()) return std::unexpected(then_result.error());

    auto attach_then =
        AttachBody(condition_node, "true", then_body, *state.graph);
    if (!attach_then.has_value()) return std::unexpected(attach_then.error());

    if (Check(state, "else")) {
        Advance(state);
        std::vector<core::NodeBase *> else_body;
        auto else_result = ParseBlock(state, &else_body);
        if (!else_result.has_value()) {
            return std::unexpected(else_result.error());
        }

        auto attach_else =
            AttachBody(condition_node, "false", else_body, *state.graph);
        if (!attach_else.has_value()) {
            return std::unexpected(attach_else.error());
        }
    }

    return condition_node;
}

std::expected<core::NodeBase *, std::string> GraphImporter::ParseWhileStatement(
    ParseState &state) const {
    Advance(state);  // 'while'
    auto open = Expect(state, "(");
    if (!open.has_value()) return std::unexpected(open.error());

    auto condition = ParseExpression(state);
    if (!condition.has_value()) return std::unexpected(condition.error());

    auto close = Expect(state, ")");
    if (!close.has_value()) return std::unexpected(close.error());

    core::NodeBase *loop_node = state.graph->AddNode<core::LoopNode>(
        core::NodeBase::NodeKind::kLoop, NextPosition(state));
    if (loop_node == nullptr) {
        return std::unexpected(ErrorAt(state, "failed to create LoopNode"));
    }

    auto cond_pin = FindInputPin(*loop_node, "cond");
    if (!cond_pin.has_value()) return std::unexpected(cond_pin.error());
    state.graph->Link(condition.value(), 0, loop_node, cond_pin.value());

    std::vector<core::NodeBase *> body;
    auto body_result = ParseBlock(state, &body);
    if (!body_result.has_value()) return std::unexpected(body_result.error());

    auto attach = AttachBody(loop_node, "body", body, *state.graph);
    if (!attach.has_value()) return std::unexpected(attach.error());

    return loop_node;
}

std::expected<core::NodeBase *, std::string> GraphImporter::ParseForStatement(
    ParseState &state) const {
    Advance(state);  // 'for'
    auto open = Expect(state, "(");
    if (!open.has_value()) return std::unexpected(open.error());

    // init: expects a declaration ("int i = 0;")
    auto init = ParseDeclaration(state);
    if (!init.has_value()) return std::unexpected(init.error());

    auto condition = ParseExpression(state);
    if (!condition.has_value()) return std::unexpected(condition.error());
    auto semi = Expect(state, ";");
    if (!semi.has_value()) return std::unexpected(semi.error());

    auto increment = ParseUpdateExpression(state);
    if (!increment.has_value()) return std::unexpected(increment.error());

    auto close = Expect(state, ")");
    if (!close.has_value()) return std::unexpected(close.error());

    core::NodeBase *for_node = state.graph->AddNode<core::ForNode>(
        core::NodeBase::NodeKind::kFor, NextPosition(state));
    if (for_node == nullptr) {
        return std::unexpected(ErrorAt(state, "failed to create ForNode"));
    }

    auto init_pin = FindInputPin(*for_node, "init");
    if (!init_pin.has_value()) return std::unexpected(init_pin.error());
    state.graph->Link(init.value(), 0, for_node, init_pin.value());

    auto cond_pin = FindInputPin(*for_node, "cond");
    if (!cond_pin.has_value()) return std::unexpected(cond_pin.error());
    state.graph->Link(condition.value(), 0, for_node, cond_pin.value());

    auto inc_pin = FindInputPin(*for_node, "step");
    if (!inc_pin.has_value()) return std::unexpected(inc_pin.error());
    state.graph->Link(increment.value(), 0, for_node, inc_pin.value());

    std::vector<core::NodeBase *> body;
    auto body_result = ParseBlock(state, &body);
    if (!body_result.has_value()) return std::unexpected(body_result.error());

    auto attach = AttachBody(for_node, "body", body, *state.graph);
    if (!attach.has_value()) return std::unexpected(attach.error());

    return for_node;
}

std::expected<core::NodeBase *, std::string>
GraphImporter::ParseUpdateExpression(ParseState &state) const {
    if (!CheckType(state, TokenType::kIdentifier)) {
        return std::unexpected(
            ErrorAt(state,
                    "expected a variable update (e.g. 'i++', 'i += 1', "
                    "'i = i + 1')"));
    }
    std::string name = Advance(state).text;

    auto it = state.symbol_table.find(name);
    if (it == state.symbol_table.end()) {
        return std::unexpected(
            ErrorAt(state, "use of undeclared variable '" + name + "'"));
    }
    core::NodeBase *current_value = it->second;

    if (Check(state, "++") || Check(state, "--")) {
        std::string op = Advance(state).text == "++" ? "+" : "-";
        auto one = MakeLiteral(state, core::NodeBase::PinDataType::kInt, "1");
        if (!one.has_value()) return std::unexpected(one.error());
        return MakeOperator(state, op, current_value, one.value());
    }

    if (Check(state, "+=") || Check(state, "-=")) {
        std::string op = Advance(state).text == "+=" ? "+" : "-";
        auto rhs = ParseExpression(state);
        if (!rhs.has_value()) return std::unexpected(rhs.error());
        return MakeOperator(state, op, current_value, rhs.value());
    }

    if (Check(state, "=")) {
        Advance(state);
        // `i = <expr>` in the increment clause - the expression may itself
        // reference `i`'s current value via the symbol table.
        return ParseExpression(state);
    }

    return std::unexpected(ErrorAt(
        state, "unsupported update operator '" + Peek(state).text + "'"));
}

int GraphImporter::BinaryPrecedence(const std::string &op) {
    if (op == "||") return 1;
    if (op == "&&") return 2;
    if (op == "==" || op == "!=") return 3;
    if (op == "<" || op == "<=" || op == ">" || op == ">=") return 4;
    if (op == "+" || op == "-") return 5;
    if (op == "*" || op == "/" || op == "%") return 6;
    return -1;
}

bool GraphImporter::IsRightAssociative(const std::string & /*op*/) {
    return false;  // every supported binary operator is left-associative
}

std::expected<core::NodeBase *, std::string> GraphImporter::ParseExpression(
    ParseState &state, int min_precedence) const {
    auto lhs = ParseUnary(state);
    if (!lhs.has_value()) return std::unexpected(lhs.error());
    core::NodeBase *left = lhs.value();

    while (true) {
        const Token &tok = Peek(state);
        if (tok.type != TokenType::kOperator) break;

        int prec = BinaryPrecedence(tok.text);
        if (prec < min_precedence) break;

        std::string op = Advance(state).text;
        int next_min = IsRightAssociative(op) ? prec : prec + 1;

        auto rhs = ParseExpression(state, next_min);
        if (!rhs.has_value()) return std::unexpected(rhs.error());

        auto combined = MakeOperator(state, op, left, rhs.value());
        if (!combined.has_value()) return std::unexpected(combined.error());
        left = combined.value();
    }

    return left;
}

std::expected<core::NodeBase *, std::string> GraphImporter::ParseUnary(
    ParseState &state) const {
    if (Check(state, "-")) {
        Advance(state);
        auto operand = ParseUnary(state);
        if (!operand.has_value()) return std::unexpected(operand.error());

        // OperatorType has no dedicated unary negate, so `-x` becomes
        // `0 - x` via the binary Subtraction operator.
        auto zero = MakeLiteral(state, core::NodeBase::PinDataType::kInt, "0");
        if (!zero.has_value()) return std::unexpected(zero.error());
        return MakeOperator(state, "-", zero.value(), operand.value());
    }

    if (Check(state, "!")) {
        Advance(state);
        auto operand = ParseUnary(state);
        if (!operand.has_value()) return std::unexpected(operand.error());
        return MakeUnaryOperator(state,
                                 core::OperatorNode::OperatorType::kLogicalNot,
                                 operand.value());
    }

    return ParsePrimary(state);
}

std::expected<core::NodeBase *, std::string> GraphImporter::ParsePrimary(
    ParseState &state) const {
    const Token &tok = Peek(state);

    if (tok.type == TokenType::kIntLiteral) {
        Advance(state);
        return MakeLiteral(state, core::NodeBase::PinDataType::kInt, tok.text);
    }
    if (tok.type == TokenType::kFloatLiteral) {
        Advance(state);
        return MakeLiteral(state, core::NodeBase::PinDataType::kFloat,
                           tok.text);
    }
    if (tok.type == TokenType::kBoolLiteral) {
        Advance(state);
        return MakeLiteral(state, core::NodeBase::PinDataType::kBool, tok.text);
    }
    if (tok.type == TokenType::kStringLiteral) {
        Advance(state);
        return MakeLiteral(state, core::NodeBase::PinDataType::kString,
                           tok.text);
    }

    if (Check(state, "(")) {
        Advance(state);
        auto inner = ParseExpression(state);
        if (!inner.has_value()) return std::unexpected(inner.error());
        auto close = Expect(state, ")");
        if (!close.has_value()) return std::unexpected(close.error());
        return inner.value();
    }

    if (tok.type == TokenType::kIdentifier) {
        Advance(state);
        auto it = state.symbol_table.find(tok.text);
        if (it == state.symbol_table.end()) {
            return std::unexpected(ErrorAt(
                state, "use of undeclared variable '" + tok.text + "'"));
        }
        return it->second;
    }

    return std::unexpected(
        ErrorAt(state, "unexpected token '" + tok.text + "' in expression"));
}

std::expected<core::NodeBase *, std::string> GraphImporter::MakeLiteral(
    ParseState &state, core::NodeBase::PinDataType type,
    const std::string &text) const {
    core::LiteralNode *literal = state.graph->AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral, NextPosition(state));
    if (literal == nullptr) {
        return std::unexpected(ErrorAt(state, "failed to create LiteralNode"));
    }

    // SetType() throws if pins are already connected, here its safe because the
    // node is newly created and unlinked.
    literal->SetType(type);
    literal->SetName(text);

    try {
        switch (type) {
            case core::NodeBase::PinDataType::kInt:
                literal->set_data(std::any(std::stoi(text)));
                break;
            case core::NodeBase::PinDataType::kFloat:
                literal->set_data(std::any(std::stod(text)));
                break;
            case core::NodeBase::PinDataType::kBool:
                literal->set_data(std::any(text == "true"));
                break;
            case core::NodeBase::PinDataType::kString:
                literal->set_data(std::any(text));
                break;
            default:
                return std::unexpected(
                    ErrorAt(state, "unsupported literal type"));
        }
    } catch (const std::exception &e) {
        return std::unexpected(ErrorAt(
            state, "failed to parse literal '" + text + "': " + e.what()));
    }

    return literal;
}

std::expected<core::NodeBase *, std::string> GraphImporter::MakeVariable(
    ParseState &state, core::NodeBase::PinDataType type,
    const std::string &name, core::NodeBase *initializer) const {
    core::VariableNode *variable = state.graph->AddNode<core::VariableNode>(
        core::NodeBase::NodeKind::kVariable, NextPosition(state));
    if (variable == nullptr) {
        return std::unexpected(ErrorAt(state, "failed to create VariableNode"));
    }

    variable->SetName(name);
    variable->SetType(type);

    state.graph->Link(initializer, 0, variable, 0);

    state.symbol_table[name] = variable;
    return variable;
}

std::expected<core::OperatorNode::OperatorType, std::string>
GraphImporter::BinaryOpToOperatorType(const std::string &op) {
    using Op = core::OperatorNode::OperatorType;
    if (op == "+") return Op::kAddition;
    if (op == "-") return Op::kSubtraction;
    if (op == "*") return Op::kMultiplication;
    if (op == "/") return Op::kDivision;
    if (op == "%") return Op::kModulo;
    if (op == "==") return Op::kEqual;
    if (op == "!=") return Op::kNotEqual;
    if (op == "<") return Op::kLessThan;
    if (op == "<=") return Op::kLessOrEqual;
    if (op == ">") return Op::kGreaterThan;
    if (op == ">=") return Op::kGreaterOrEqual;
    if (op == "&&") return Op::kLogicalAnd;
    if (op == "||") return Op::kLogicalOr;
    return std::unexpected("unsupported binary operator '" + op + "'");
}

std::expected<core::NodeBase *, std::string> GraphImporter::MakeOperator(
    ParseState &state, const std::string &op, core::NodeBase *lhs,
    core::NodeBase *rhs) const {
    auto op_type = BinaryOpToOperatorType(op);
    if (!op_type.has_value()) {
        return std::unexpected(ErrorAt(state, op_type.error()));
    }

    core::OperatorNode *operator_node =
        state.graph->AddNode<core::OperatorNode>(
            core::NodeBase::NodeKind::kOperator, NextPosition(state));
    if (operator_node == nullptr) {
        return std::unexpected(ErrorAt(state, "failed to create OperatorNode"));
    }

    operator_node->SetOperatorType(op_type.value());
    operator_node->SetName(op);

    state.graph->Link(lhs, 0, operator_node, 0);
    state.graph->Link(rhs, 0, operator_node, 1);

    return operator_node;
}

std::expected<core::NodeBase *, std::string> GraphImporter::MakeUnaryOperator(
    ParseState &state, core::OperatorNode::OperatorType type,
    core::NodeBase *operand) const {
    core::OperatorNode *operator_node =
        state.graph->AddNode<core::OperatorNode>(
            core::NodeBase::NodeKind::kOperator, NextPosition(state));
    if (operator_node == nullptr) {
        return std::unexpected(ErrorAt(state, "failed to create OperatorNode"));
    }

    operator_node->SetOperatorType(type);
    operator_node->SetName(core::OperatorTypeToString(type));

    // Unary operators only use input pin 0 ("Input"); pin 1 is left
    // unconnected (harmless leftover slot from the fixed binary-sized
    // parents_ vector - see MakeOperator's comment).
    state.graph->Link(operand, 0, operator_node, 0);

    return operator_node;
}

std::expected<void, std::string> GraphImporter::AttachBody(
    core::NodeBase *control_node, const std::string &body_pin_name,
    const std::vector<core::NodeBase *> &body_nodes, core::Graph &graph) const {
    if (body_nodes.empty()) return {};

    auto pin = FindOutputPin(*control_node, body_pin_name);
    if (!pin.has_value()) return std::unexpected(pin.error());

    for (core::NodeBase *statement : body_nodes) {
        // Only PrintNode exposes a "control" (kVoid) input pin to attach
        // through. Declarations (VariableNode) inside a block have no such
        // pin - they're left for the topological/data-dependency pass to
        // place near their point of use. This split is the part that most
        // needs confirming against CodegenContext's actual deferral logic
        // (see the doc's "the generator can still defer some nodes when
        // they belong to the body of a control block").
        if (statement->kind() != core::NodeBase::NodeKind::kPrint) {
            continue;
        }

        auto control_pin = FindInputPin(*statement, "control");
        if (!control_pin.has_value()) {
            return std::unexpected(control_pin.error());
        }

        // Multiple PrintNodes can fan out from the same void output pin;
        // their relative order is assumed to follow connection insertion
        // order (Graph::Link()/AddChild() push_back onto the pin's
        // connections vector) rather than any explicit chain, since
        // PrintNode itself has no output pin to chain through.
        graph.Link(control_node, pin.value(), statement, control_pin.value());
    }
    return {};
}

std::expected<uint8_t, std::string> GraphImporter::FindInputPin(
    const core::NodeBase &node, const std::string &name) {
    for (uint8_t i = 0; i < node.GetInputPinCount(); ++i) {
        if (node.GetInputPinName(i) == name) return i;
    }
    return std::unexpected("node has no input pin named '" + name + "'");
}

std::expected<uint8_t, std::string> GraphImporter::FindOutputPin(
    const core::NodeBase &node, const std::string &name) {
    for (uint8_t i = 0; i < node.GetOutputPinCount(); ++i) {
        if (node.GetOutputPinName(i) == name) return i;
    }
    return std::unexpected("node has no output pin named '" + name + "'");
}

bool GraphImporter::IsTypeKeyword(const std::string &text) {
    return text == "int" || text == "float" || text == "double" ||
           text == "bool" || text == "string";
}

core::NodeBase::PinDataType GraphImporter::KeywordToPinType(
    const std::string &text) {
    if (text == "int") {
        return core::NodeBase::PinDataType::kInt;
    }
    if (text == "float" || text == "double") {
        return core::NodeBase::PinDataType::kFloat;
    }
    if (text == "bool") {
        return core::NodeBase::PinDataType::kBool;
    }
    if (text == "string") {
        return core::NodeBase::PinDataType::kString;
    }
    return core::NodeBase::PinDataType::kUndefined;
}

}  // namespace graph_importer