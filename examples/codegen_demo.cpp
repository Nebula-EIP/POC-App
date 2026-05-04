#include <iostream>
#include <string>

#include "src/core/graph.hpp"
#include "src/core/nodes/literal_node.hpp"
#include "src/core/nodes/operator_node.hpp"
#include "src/editor/code-generation/codegen-context.hpp"

int main() {
    // Create a simple graph: (3 + 4) * 2
    core::Graph g;
    
    auto *lit1 = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto *lit2 = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto *lit3 = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    
    auto *add_op = g.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator);
    auto *mult_op = g.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator);
    
    // Set literal values
    lit1->set_data(3);
    lit2->set_data(4);
    lit3->set_data(2);
    
    // Set operator types
    add_op->set_operator_type(core::OperatorNode::OperatorType::kAddition);
    mult_op->set_operator_type(core::OperatorNode::OperatorType::kMultiplication);
    
    // Connect nodes
    g.Link(lit1, 0, add_op, 0);
    g.Link(lit2, 0, add_op, 1);
    g.Link(add_op, 0, mult_op, 0);
    g.Link(lit3, 0, mult_op, 1);
    
    // Generate code WITHOUT outputs
    std::cout << "===== STANDARD GENERATION =====" << std::endl;
    editor::code_generation::CodegenContext ctx;
    auto file1 = ctx.Generate(g);
    std::cout << file1.GetFormatedContent() << std::endl;
    
    std::cout << "\n===== WITH OUTPUTS =====" << std::endl;
    // Generate code WITH outputs
    auto file2 = ctx.GenerateWithOutputs(g, true);
    std::cout << file2.GetFormatedContent() << std::endl;
    
    return 0;
}
