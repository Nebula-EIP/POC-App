#include "graph-exporter.hpp"
#include "core/nodes/variable_node.hpp"
#include <functional>

std::string GraphExporter::ParseVariableNode(const core::NodeBase *node) {
    const auto *variable_node = dynamic_cast<const core::VariableNode *>(node);
    if (!variable_node) {
        return "";
    }

    std::string code_line = "";
    std::string value_str = "";
    switch (variable_node->type()) {
        case core::NodeBase::PinDataType::kInt: {
            code_line = "int ";
            try {
                int idata = std::any_cast<int>(variable_node->data());
                value_str = std::to_string(idata);
            } catch (...) {}
            break;
        }
        case core::NodeBase::PinDataType::kFloat: {
            code_line = "float ";
            try {
                float fdata = std::any_cast<float>(variable_node->data());
                value_str = std::to_string(fdata) + "f";
            } catch (...) {}
            break;
        }
        case core::NodeBase::PinDataType::kBool: {
            code_line = "bool ";
            try {
                bool bdata = std::any_cast<bool>(variable_node->data());
                value_str = bdata ? "true" : "false";
            } catch (...) {}
            break;
        }
        case core::NodeBase::PinDataType::kString: {
            code_line = "std::string ";
            try {
                std::string sdata = std::any_cast<std::string>(variable_node->data());
                value_str = "\"" + sdata + "\"";
            } catch (...) {}
            break;
        }
        default:
            return "";
    }
    if (value_str.empty()) {
        return code_line + variable_node->name() + ";";
    } else {
        return code_line + variable_node->name() + " = " + value_str + ";";
    }
}

// TO DO : faire que ca 