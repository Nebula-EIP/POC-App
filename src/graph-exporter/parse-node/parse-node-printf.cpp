#include "graph-exporter.hpp"

#include "core/nodes/printf_node.hpp"

std::string GraphExporter::ParsePrintfNode(const core::NodeBase *node) {
    const auto *printf_node = dynamic_cast<const core::PrintfNode *>(node);
    if (!printf_node) {
        return "";
    }

    std::string value_expr;
    if (node->parent(0)) {
        value_expr = node->parent(0)->node->GetDisplayName();
    } else {
        return "";
    }

    std::string format_specifier;
    switch (printf_node->type()) {
        case core::NodeBase::PinDataType::kInt:
            format_specifier = "%d";
            break;
        case core::NodeBase::PinDataType::kFloat:
            format_specifier = "%f";
            break;
        case core::NodeBase::PinDataType::kBool:
            format_specifier = "%d";
            break;
        case core::NodeBase::PinDataType::kString:
            format_specifier = "%s";
            value_expr = value_expr + ".c_str()"; // Convert std::string to C-style string for printf
            break;
        default:
            return "";
    }

    return "printf(\"" + format_specifier + "\\n\", " + value_expr + ");";
}
