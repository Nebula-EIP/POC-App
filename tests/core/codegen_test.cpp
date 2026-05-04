#include <gtest/gtest.h>

#include "graph.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/operator_node.hpp"
#include "../../src/editor/code-generation/codegen-context.hpp"

TEST(CodegenTest, SimpleAddProducesCpp) {
    core::Graph g;
    auto *l1 = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto *l2 = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto *op = g.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator);

    l1->set_data(3);
    l2->set_data(4);

    g.Link(l1, 0, op, 0);
    g.Link(l2, 0, op, 1);

    editor::code_generation::CodegenContext ctx;
    auto file = ctx.Generate(g);
    auto content = file.GetFormatedContent();
    EXPECT_NE(content.find("auto v" + std::to_string(l1->id())), std::string::npos);
    EXPECT_NE(content.find("auto v" + std::to_string(op->id())), std::string::npos);
    EXPECT_NE(content.find("3"), std::string::npos);
    EXPECT_NE(content.find("4"), std::string::npos);
}
