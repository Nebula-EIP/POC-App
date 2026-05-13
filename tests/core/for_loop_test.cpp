#include <gtest/gtest.h>

#include <string>

#include "core/graph.hpp"
#include "core/nodes/for_node.hpp"
#include "core/nodes/literal_node.hpp"
#include "core/nodes/operator_node.hpp"
#include "core/nodes/print_node.hpp"
#include "core/nodes/variable_node.hpp"
#include "editor/code-generation/codegen-context.hpp"

TEST(ForLoopTest, GeneratesForLoopWithBodyAndLoopVariableReference) {
    core::Graph graph;

    auto *init = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral,
                                                  {0, 0});
    auto *limit = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral,
                                                   {0, 0});
    auto *step = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral,
                                                 {0, 0});
    auto *loop = graph.AddNode<core::ForNode>(core::NodeBase::NodeKind::kFor,
                                              {0, 0});
    auto *loop_var = graph.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable,
                                                       {0, 0});
    auto *multiplier = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral,
                                                        {0, 0});
    auto *compare = graph.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator,
                                                      {0, 0});
    auto *product = graph.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator,
                                                     {0, 0});
    auto *printer = graph.AddNode<core::PrintNode>(core::NodeBase::NodeKind::kPrint,
                                                  {0, 0});

    init->set_data(1);
    limit->set_data(4);
    step->set_data(1);
    multiplier->set_data(2);
    loop->SetName("i");
    loop_var->SetName("i");
    loop_var->SetType(core::NodeBase::PinDataType::kInt);

    compare->SetOperatorType(core::OperatorNode::OperatorType::kLessOrEqual);
    product->SetOperatorType(core::OperatorNode::OperatorType::kMultiplication);

    try { graph.Link(init, 0, loop, 0); } catch (const std::exception &e) { FAIL() << "init->loop: " << e.what(); }
    try { graph.Link(compare, 0, loop, 1); } catch (const std::exception &e) { FAIL() << "compare->loop: " << e.what(); }
    try { graph.Link(step, 0, loop, 2); } catch (const std::exception &e) { FAIL() << "step->loop: " << e.what(); }

    try { graph.Link(loop_var, 0, compare, 0); } catch (const std::exception &e) { FAIL() << "loop_var->compare: " << e.what(); }
    try { graph.Link(limit, 0, compare, 1); } catch (const std::exception &e) { FAIL() << "limit->compare: " << e.what(); }

    try { graph.Link(loop_var, 0, product, 0); } catch (const std::exception &e) { FAIL() << "loop_var->product: " << e.what(); }
    try { graph.Link(multiplier, 0, product, 1); } catch (const std::exception &e) { FAIL() << "multiplier->product: " << e.what(); }

    try { graph.Link(loop, 0, printer, 0); } catch (const std::exception &e) { FAIL() << "loop->printer control: " << e.what(); }
    try { graph.Link(product, 0, printer, 1); } catch (const std::exception &e) { FAIL() << "product->printer value: " << e.what(); }

    editor::code_generation::CodegenContext context;
    const auto content = context.Generate(graph).GetFormatedContent();

    EXPECT_NE(content.find("for (int i = 1;"), std::string::npos);
    EXPECT_NE(content.find("i <= lit_"), std::string::npos);
    EXPECT_NE(content.find("std::cout << i * lit_"), std::string::npos);
}
