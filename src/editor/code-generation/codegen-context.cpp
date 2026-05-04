#include "codegen-context.hpp"

#include <sstream>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <optional>

#include "core/graph.hpp"
#include "core/node_base.hpp"
#include "core/nodes/literal_node.hpp"
#include "core/nodes/operator_node.hpp"

static std::string LiteralToCpp(const core::LiteralNode &lit) {
    using PD = core::NodeBase::PinDataType;
    std::ostringstream oss;
    switch (lit.type()) {
        case PD::kInt: {
            try {
                int v = std::any_cast<int>(lit.data());
                oss << v;
            } catch (...) {
                oss << "0";
            }
            break;
        }
        case PD::kFloat: {
            try {
                double v = std::any_cast<double>(lit.data());
                oss << v;
            } catch (...) {
                oss << "0.0";
            }
            break;
        }
        case PD::kString: {
            try {
                std::string s = std::any_cast<std::string>(lit.data());
                oss << '"' << s << '"';
            } catch (...) {
                oss << '"' << "" << '"';
            }
            break;
        }
        case PD::kBool: {
            try {
                bool b = std::any_cast<bool>(lit.data());
                oss << (b ? "true" : "false");
            } catch (...) {
                oss << "false";
            }
            break;
        }
        default:
            oss << "0";
    }
    return oss.str();
}

static std::string OpSymbol(core::OperatorNode::OperatorType t) {
    using OT = core::OperatorNode::OperatorType;
    switch (t) {
        case OT::kAddition: return "+";
        case OT::kSubtraction: return "-";
        case OT::kMultiplication: return "*";
        case OT::kDivision: return "/";
        case OT::kModulo: return "%";
        case OT::kBitwiseAnd: return "&";
        case OT::kBitwiseOr: return "|";
        case OT::kBitwiseXor: return "^";
        case OT::kLeftShift: return "<<";
        case OT::kRightShift: return ">>";
        case OT::kEqual: return "==";
        case OT::kNotEqual: return "!=";
        case OT::kLessThan: return "<";
        case OT::kGreaterThan: return ">";
        case OT::kLessOrEqual: return "<=";
        case OT::kGreaterOrEqual: return ">=";
        case OT::kLogicalAnd: return "&&";
        case OT::kLogicalOr: return "||";
        default: return "+";
    }
}

::code_generation::CodeGeneratorFile editor::code_generation::CodegenContext::Generate(const core::Graph &graph) {
    ::code_generation::CodeGeneratorFile file;

    file.Line("#include <iostream>");
    file.Line("");
    file.Line("int main() {");

    // Build a topo order (Kahn) based on input connections. Falls back to
    // insertion order for cycles or disconnected graphs.
    std::vector<const core::NodeBase *> nodes;
    nodes.reserve(graph.GetAllNodes().size());
    for (const auto &nptr : graph.GetAllNodes()) nodes.push_back(nptr.get());

    std::unordered_map<uint32_t, int> indeg;
    for (const auto *n : nodes) indeg[n->id()] = 0;

    for (const auto *n : nodes) {
        for (const auto &p : n->GetAllParents()) {
            if (p.IsConnected() && p.node) indeg[n->id()]++;
        }
    }

    std::queue<const core::NodeBase *> q;
    for (const auto *n : nodes) if (indeg[n->id()] == 0) q.push(n);

    std::vector<const core::NodeBase *> order;
    while (!q.empty()) {
        const core::NodeBase *n = q.front(); q.pop();
        order.push_back(n);
        for (const auto &conn : n->GetAllChildrens()) {
            if (!conn.IsConnected() || !conn.node) continue;
            auto &did = indeg[conn.node->id()];
            did--;
            if (did == 0) q.push(conn.node);
        }
    }

    // If cycle detected (order size < nodes), append remaining nodes in
    // insertion order to ensure all nodes are emitted.
    if (order.size() < nodes.size()) {
        for (const auto *n : nodes) {
            if (std::find(order.begin(), order.end(), n) == order.end())
                order.push_back(n);
        }
    }

    // Symbol table and constant folding for integer literals.
    std::unordered_map<uint32_t, std::string> sym;
    std::unordered_map<uint32_t, long long> const_int;

    auto GetLiteralInt = [&](const core::NodeBase *n) -> std::optional<long long> {
        if (n->kind() != core::NodeBase::NodeKind::kLiteral) return std::nullopt;
        auto *lit = static_cast<const core::LiteralNode *>(n);
        if (lit->type() != core::NodeBase::PinDataType::kInt) return std::nullopt;
        try {
            int v = std::any_cast<int>(lit->data());
            return static_cast<long long>(v);
        } catch (...) {
            return std::nullopt;
        }
    };

    auto TryFoldOperatorInt = [&](const core::OperatorNode *op, long long &out) -> bool {
        // binary operators only
        try {
            auto p0 = op->parent(0);
            auto p1 = op->parent(1);
            if (!p0.IsConnected() || !p1.IsConnected()) return false;
            auto *n0 = p0.node;
            auto *n1 = p1.node;

            auto it0 = const_int.find(n0->id());
            auto it1 = const_int.find(n1->id());
            if (it0 == const_int.end() || it1 == const_int.end()) return false;

            long long a = it0->second;
            long long b = it1->second;
            using OT = core::OperatorNode::OperatorType;
            switch (op->operator_type()) {
                case OT::kAddition: out = a + b; return true;
                case OT::kSubtraction: out = a - b; return true;
                case OT::kMultiplication: out = a * b; return true;
                case OT::kDivision: if (b == 0) return false; out = a / b; return true;
                case OT::kModulo: if (b == 0) return false; out = a % b; return true;
                default: return false;
            }
        } catch (...) {
            return false;
        }
    };

    for (const auto *n : order) {
        if (n->kind() == core::NodeBase::NodeKind::kLiteral) {
            auto *lit = static_cast<const core::LiteralNode *>(n);
            std::ostringstream ln;
            ln << "auto lit_" << lit->id() << " = " << LiteralToCpp(*lit) << ";";
            file.Line("    " + ln.str());

            if (lit->type() == core::NodeBase::PinDataType::kInt) {
                if (auto v = GetLiteralInt(n)) {
                    const_int[n->id()] = *v;
                    sym[n->id()] = std::to_string(*v);
                }
            } else {
                sym[n->id()] = "lit_" + std::to_string(n->id());
            }
        } else if (n->kind() == core::NodeBase::NodeKind::kOperator) {
            auto *op = static_cast<const core::OperatorNode *>(n);

            long long folded = 0;
            if (TryFoldOperatorInt(op, folded)) {
                // produce constant symbol
                const_int[op->id()] = folded;
                sym[op->id()] = std::to_string(folded);
                // no variable emitted when folded
                continue;
            }

            // Not folded: emit readable temporary name
            std::string a = "v" + std::to_string(op->parent(0).node->id());
            std::string b = "v" + std::to_string(op->parent(1).node->id());
            // prefer symbolic names if available
            if (sym.find(op->parent(0).node->id()) != sym.end()) a = sym[op->parent(0).node->id()];
            if (sym.find(op->parent(1).node->id()) != sym.end()) b = sym[op->parent(1).node->id()];

            std::ostringstream ln;
            ln << "auto tmp_" << op->id() << " = " << a << " " << OpSymbol(op->operator_type()) << " " << b << ";";
            file.Line("    " + ln.str());
            sym[op->id()] = "tmp_" + std::to_string(op->id());
        } else {
            // fallback: create generic symbol
            sym[n->id()] = "v" + std::to_string(n->id());
        }
    }

    file.Line("");
    file.Line("    return 0;");
    file.Line("}");

    return file;
}
