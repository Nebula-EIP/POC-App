#include "graph-exporter.hpp"
#include "core/nodes/literal_node.hpp"
#include <functional>
#include <any>

std::string GraphExporter::ParseLiteralNode(const core::NodeBase *node) {
    const auto *literal_node = dynamic_cast<const core::LiteralNode *>(node);
    if (!literal_node) {
        return "";
    }

    std::string code_line = "";
    std::string value_str = "";
    switch (literal_node->type()) {
        case core::NodeBase::PinDataType::kInt: {
            int idata = std::any_cast<int>(literal_node->data());
            code_line = "int ";
            value_str = std::to_string(idata);
            break;
        }
        case core::NodeBase::PinDataType::kFloat: {
            float fdata = std::any_cast<float>(literal_node->data());
            code_line = "float ";
            value_str = std::to_string(fdata) + "f";
            break;
        }
        case core::NodeBase::PinDataType::kBool: {
            bool bdata = std::any_cast<bool>(literal_node->data());
            code_line = "bool ";
            value_str = bdata ? "true" : "false";
            break;
        }
        case core::NodeBase::PinDataType::kString: {
            std::string sdata = std::any_cast<std::string>(literal_node->data());
            code_line = "std::string ";
            value_str = "\"" + sdata + "\"";
            break;
        }
        default:
            return "";
    }
    return "const " + code_line + literal_node->name() + " = " + value_str + ";";
}