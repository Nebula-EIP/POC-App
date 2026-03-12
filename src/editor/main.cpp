#include <iostream>

#include "graph.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/variable_node.hpp"
#include "nodes/printf_node.hpp"
#include "graph-exporter/graph-exporter.hpp"
#include "file-writing/file-writer.hpp"

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

    // Create printf nodes to display values
    auto *print_int =
        graph.AddNode<core::PrintfNode>(core::NodeBase::NodeKind::kPrintf);
    print_int->set_type(core::NodeBase::PinDataType::kInt);

    auto *print_float =
        graph.AddNode<core::PrintfNode>(core::NodeBase::NodeKind::kPrintf);
    print_float->set_type(core::NodeBase::PinDataType::kFloat);

    auto *print_str =
        graph.AddNode<core::PrintfNode>(core::NodeBase::NodeKind::kPrintf);
    print_str->set_type(core::NodeBase::PinDataType::kString);

    // Link variables/literals to printf nodes
    auto link3 = graph.Link(int_var, 0, print_int, 0);
    if (!link3) {
        std::cerr << "Link error: " << link3.error() << std::endl;
        return 1;
    }

    auto link4 = graph.Link(float_var, 0, print_float, 0);
    if (!link4) {
        std::cerr << "Link error: " << link4.error() << std::endl;
        return 1;
    }

    auto link5 = graph.Link(str_literal, 0, print_str, 0);
    if (!link5) {
        std::cerr << "Link error: " << link5.error() << std::endl;
        return 1;
    }

    GraphExporter exporter;
    code_generation::CodeGeneratorFile cgf = exporter.ExportToNebula(graph, "output.nebula");
    file_writing::FileWriter fw;
    fw.WriteToFile("code-generated/main.cpp", cgf.GetFormatedContent());
    return 0;
}
