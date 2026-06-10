#pragma once

#include <string>

#include "core/graph.hpp"
#include "core/nodes/condition_node.hpp"
#include "core/nodes/for_node.hpp"
#include "core/nodes/literal_node.hpp"
#include "core/nodes/loop_node.hpp"
#include "core/nodes/operator_node.hpp"
#include "core/nodes/print_node.hpp"
#include "core/nodes/variable_node.hpp"

inline void BuildMegaFlowGraph(core::Graph &graph) {
    auto *value_a = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral,
                                                     {0, 0});
    auto *value_b = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral,
                                                     {0, 0});
    auto *value_c = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral,
                                                     {0, 0});
    auto *value_d = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral,
                                                     {0, 0});
    auto *keep_running = graph.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable,
                                                           {0, 0});
    auto *validation_compare =
        graph.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator, {0, 0});
    auto *validation_if = graph.AddNode<core::ConditionNode>(
        core::NodeBase::NodeKind::kCondition, {0, 0});
    auto *validation_error = graph.AddNode<core::PrintNode>(core::NodeBase::NodeKind::kPrint,
                                                            {0, 0});
    auto *validation_ok = graph.AddNode<core::PrintNode>(core::NodeBase::NodeKind::kPrint,
                                                         {0, 0});
    auto *init_value = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral,
                                                        {0, 0});
    auto *step_value = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral,
                                                        {0, 0});
    auto *loop_node = graph.AddNode<core::ForNode>(core::NodeBase::NodeKind::kFor, {0, 0});
    auto *loop_var = graph.AddNode<core::VariableNode>(core::NodeBase::NodeKind::kVariable,
                                                       {0, 0});
    auto *loop_compare = graph.AddNode<core::OperatorNode>(
        core::NodeBase::NodeKind::kOperator, {0, 0});
    auto *loop_product = graph.AddNode<core::OperatorNode>(
        core::NodeBase::NodeKind::kOperator, {0, 0});
    auto *loop_print = graph.AddNode<core::PrintNode>(core::NodeBase::NodeKind::kPrint,
                                                      {0, 0});
    auto *while_node = graph.AddNode<core::LoopNode>(core::NodeBase::NodeKind::kLoop,
                                                    {0, 0});
    auto *while_body_print = graph.AddNode<core::PrintNode>(core::NodeBase::NodeKind::kPrint,
                                                            {0, 0});
    auto *sum = graph.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator,
                                                  {0, 0});
    auto *difference = graph.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator,
                                                         {0, 0});
    auto *product = graph.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator,
                                                      {0, 0});
    auto *quotient = graph.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator,
                                                       {0, 0});
    auto *final_print = graph.AddNode<core::PrintNode>(core::NodeBase::NodeKind::kPrint,
                                                       {0, 0});
    auto *validation_error_text =
        graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *validation_ok_text =
        graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *while_body_text = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral,
                                                             {0, 0});
    auto *loop_limit = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral,
                                                        {0, 0});

    value_a->set_data(10);
    value_b->set_data(2);
    value_c->set_data(4);
    value_d->set_data(3);

    keep_running->SetType(core::NodeBase::PinDataType::kBool);
    keep_running->SetName("keep_running");
    keep_running->set_data(false);

    loop_var->SetType(core::NodeBase::PinDataType::kInt);
    loop_var->SetName("i");
    loop_node->SetName("i");

    validation_error_text->SetType(core::NodeBase::PinDataType::kString);
    validation_error_text->set_data(std::string("validation error"));
    validation_ok_text->SetType(core::NodeBase::PinDataType::kString);
    validation_ok_text->set_data(std::string("validation ok"));
    while_body_text->SetType(core::NodeBase::PinDataType::kString);
    while_body_text->set_data(std::string("while body"));
    loop_limit->set_data(4);

    validation_compare->SetOperatorType(core::OperatorNode::OperatorType::kLessThan);
    loop_compare->SetOperatorType(core::OperatorNode::OperatorType::kLessOrEqual);
    loop_product->SetOperatorType(core::OperatorNode::OperatorType::kMultiplication);
    sum->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    difference->SetOperatorType(core::OperatorNode::OperatorType::kSubtraction);
    product->SetOperatorType(core::OperatorNode::OperatorType::kMultiplication);
    quotient->SetOperatorType(core::OperatorNode::OperatorType::kDivision);

    init_value->set_data(1);
    step_value->set_data(1);

    graph.Link(value_a, 0, validation_compare, 0);
    graph.Link(value_b, 0, validation_compare, 1);
    graph.Link(validation_compare, 0, validation_if, 0);
    graph.Link(validation_if, 0, validation_error, 0);
    graph.Link(validation_error_text, 0, validation_error, 1);
    graph.Link(validation_if, 1, validation_ok, 0);
    graph.Link(validation_ok_text, 0, validation_ok, 1);

    graph.Link(init_value, 0, loop_node, 0);
    graph.Link(loop_compare, 0, loop_node, 1);
    graph.Link(step_value, 0, loop_node, 2);
    graph.Link(loop_var, 0, loop_compare, 0);
    graph.Link(loop_limit, 0, loop_compare, 1);
    graph.Link(loop_var, 0, loop_product, 0);
    graph.Link(value_c, 0, loop_product, 1);
    graph.Link(loop_node, 0, loop_print, 0);
    graph.Link(loop_product, 0, loop_print, 1);

    graph.Link(keep_running, 0, while_node, 0);
    graph.Link(while_node, 0, while_body_print, 0);
    graph.Link(while_body_text, 0, while_body_print, 1);

    graph.Link(value_a, 0, sum, 0);
    graph.Link(value_b, 0, sum, 1);
    graph.Link(sum, 0, difference, 0);
    graph.Link(value_c, 0, difference, 1);
    graph.Link(difference, 0, product, 0);
    graph.Link(value_d, 0, product, 1);
    graph.Link(product, 0, quotient, 0);
    graph.Link(value_b, 0, quotient, 1);
    graph.Link(quotient, 0, final_print, 1);
}