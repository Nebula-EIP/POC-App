#include <gtest/gtest.h>

#include "core/graph.hpp"
#include "core/nodes/literal_node.hpp"
#include "core/nodes/print_node.hpp"
#include "editor/code-generation/codegen-context.hpp"

using namespace core;

TEST(PrintNodeTest, GeneratesPrintStatementForStringLiteral) {
    core::Graph g;
    auto *lit = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0,0});
    auto *print = g.AddNode<core::PrintNode>(core::NodeBase::NodeKind::kPrint, {0,0});

    lit->SetType(core::NodeBase::PinDataType::kString);
    lit->set_data(std::string("hello"));

    g.Link(lit, 0, print, 0);

    editor::code_generation::CodegenContext ctx;
    auto content = ctx.Generate(g).GetFormatedContent();

    EXPECT_NE(content.find("std::cout << \"hello\""), std::string::npos);
}
