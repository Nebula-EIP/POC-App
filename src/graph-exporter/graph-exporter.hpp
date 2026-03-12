#pragma once

#include <string>

#include "graph.hpp"
#include "code-generation/code-generator-file.hpp"

class GraphExporter {
public:
    GraphExporter();

    void ExportToNebula(const core::Graph& graph, const std::string& filename);

private:
    void ParseNode(const core::NodeBase* node, code_generation::CodeGeneratorFile& code_file);
    std::string ParseContentNodeToNebula(const core::NodeBase* node);
    std::map<core::NodeBase::NodeKind, std::function<std::string(const core::NodeBase *)>> node_parsers;

    // functions to parse different node kinds to nebula code
    std::string ParseLiteralNode(const core::NodeBase* node);
    std::string ParseVariableNode(const core::NodeBase* node);
};