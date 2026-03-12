#include "graph-exporter.hpp"
#include <functional>

GraphExporter::GraphExporter()
{
    node_parsers[core::NodeBase::NodeKind::kLiteral] = std::bind(&GraphExporter::ParseLiteralNode, this, std::placeholders::_1);
    node_parsers[core::NodeBase::NodeKind::kVariable] = std::bind(&GraphExporter::ParseVariableNode, this, std::placeholders::_1);
}


std::string GraphExporter::ParseContentNodeToNebula(const core::NodeBase *node) {
    try {
        return node_parsers[node->kind()](node);
    } catch (...) {
        return "";
    }
}

void GraphExporter::ParseNode(const core::NodeBase *node, code_generation::CodeGeneratorFile &code_file)
{
    code_file.Line(ParseContentNodeToNebula(node));
    for (int i = 0; i < node->GetOutputPinCount(); i++) {
        for (const auto &childrens : node->childrens(i)) {
            if (childrens.node)
                ParseNode(childrens.node, code_file);
        }
    }
}

void GraphExporter::ExportToNebula(const core::Graph &graph,
                                   const std::string &filename)
{
    std::string path = "code-generated/" + filename;
    code_generation::CodeGeneratorFile code_file;

    const auto &nodes = graph.GetNodes();
    std::vector<core::NodeBase*> nodes_heads;

    for (const auto &node : nodes) {
        if (node->GetParents().empty()) {
            nodes_heads.push_back(node.get());
        }
    }
    code_file.OpenBlock("int main()");
    for (const auto &head : nodes_heads)
        ParseNode(head, code_file);
    code_file.CloseAllBlocks();
    printf("Exporting graph to \n%s\n", code_file.GetFormatedContent().c_str());
}