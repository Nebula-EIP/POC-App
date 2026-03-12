#include <iostream>

#include "graph.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/variable_node.hpp"
#include "graph-exporter/graph-exporter.hpp"

int main(void) {
    core::Graph graph;
    graph.SetProjectName("Basic Graph Demo");
    graph.SetAuthor("Nebula");

    // Create literal nodes (constants) of different types
    auto *int_literal =
        graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    int_literal->set_name("IntConstant");
    int_literal->set_type(core::NodeBase::PinDataType::kInt);
    int_literal->set_data(42);

    auto *float_literal =
        graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    float_literal->set_name("FloatConstant");
    float_literal->set_type(core::NodeBase::PinDataType::kFloat);
    float_literal->set_data(3.14f);

    auto *str_literal =
        graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    str_literal->set_name("StringConstant");
    str_literal->set_type(core::NodeBase::PinDataType::kString);
    str_literal->set_data(std::string("Hello Nebula!"));

    // Create variable nodes
    auto *int_var =
        graph.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable);
    int_var->set_name("x");
    int_var->set_type(core::NodeBase::PinDataType::kInt);

    auto *float_var =
        graph.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable);
    float_var->set_name("pi");
    float_var->set_type(core::NodeBase::PinDataType::kFloat);

    // Link literal outputs to variable inputs
    auto link1 = graph.Link(int_literal, 0, int_var, 0);
    if (!link1) {
        std::cerr << "Link error: " << link1.error() << std::endl;
        return 1;
    }

    // Link float literal to float variable
    auto link2 = graph.Link(float_literal, 0, float_var, 0);
    if (!link2) {
        std::cerr << "Link error: " << link2.error() << std::endl;
        return 1;
    }

    GraphExporter exporter;
    exporter.ExportToNebula(graph, "output.nebula");

    return 0;
}
