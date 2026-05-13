#include <gtest/gtest.h>

#include "core/graph.hpp"
#include "core/nodes/literal_node.hpp"
#include "core/nodes/print_node.hpp"
#include "core/nodes/condition_node.hpp"
#include "core/nodes/loop_node.hpp"
#include "editor/code-generation/codegen-context.hpp"

using namespace core;

TEST(ControlFlowTest, IfGeneratesIfBlock) {
    core::Graph g;
    auto *lit = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0,0});
    auto *cond = g.AddNode<core::ConditionNode>(core::NodeBase::NodeKind::kCondition, {0,0});
    auto *print = g.AddNode<core::PrintNode>(core::NodeBase::NodeKind::kPrint, {0,0});

    lit->SetType(core::NodeBase::PinDataType::kBool);
    lit->set_data(true);

    g.Link(lit, 0, cond, 0); // condition input
    g.Link(cond, 0, print, 0); // true branch

    editor::code_generation::CodegenContext ctx;
    auto content = ctx.Generate(g).GetFormatedContent();

    EXPECT_NE(content.find("if (true) {"), std::string::npos);
    EXPECT_NE(content.find("std::cout"), std::string::npos);
}

TEST(ControlFlowTest, WhileGeneratesWhileBlock) {
    core::Graph g;
    auto *lit = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0,0});
    auto *loop = g.AddNode<core::LoopNode>(core::NodeBase::NodeKind::kLoop, {0,0});
    auto *print = g.AddNode<core::PrintNode>(core::NodeBase::NodeKind::kPrint, {0,0});

    lit->SetType(core::NodeBase::PinDataType::kBool);
    lit->set_data(true);

    g.Link(lit, 0, loop, 0); // condition
    g.Link(loop, 0, print, 0); // body

    editor::code_generation::CodegenContext ctx;
    auto content = ctx.Generate(g).GetFormatedContent();

    EXPECT_NE(content.find("while (true) {"), std::string::npos);
    EXPECT_NE(content.find("std::cout"), std::string::npos);
}
