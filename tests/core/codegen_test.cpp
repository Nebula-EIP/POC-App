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
    EXPECT_NE(content.find("const int lit_" + std::to_string(l1->id())), std::string::npos);
    EXPECT_NE(content.find("const int lit_" + std::to_string(l2->id())), std::string::npos);
    EXPECT_NE(content.find("const int tmp_" + std::to_string(op->id())), std::string::npos);
    EXPECT_NE(content.find("= 7;"), std::string::npos);
    EXPECT_EQ(content.find("auto "), std::string::npos);
}

TEST(CodegenTest, FloatLiteralUsesDoubleType) {
    core::Graph g;
    auto *literal = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    literal->set_type(core::NodeBase::PinDataType::kFloat);
    literal->set_data(2.5);

    editor::code_generation::CodegenContext ctx;
    auto content = ctx.Generate(g).GetFormatedContent();
    EXPECT_NE(content.find("const double lit_" + std::to_string(literal->id())), std::string::npos);
    EXPECT_NE(content.find("2.5"), std::string::npos);
}

TEST(CodegenTest, StringLiteralUsesStdStringType) {
    core::Graph g;
    auto *literal = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    literal->set_type(core::NodeBase::PinDataType::kString);
    literal->set_data(std::string("hello"));

    editor::code_generation::CodegenContext ctx;
    auto content = ctx.Generate(g).GetFormatedContent();
    EXPECT_NE(content.find("const std::string lit_" + std::to_string(literal->id())), std::string::npos);
    EXPECT_NE(content.find("\"hello\""), std::string::npos);
}

TEST(CodegenTest, LogicalNotFoldedFromBoolLiteral) {
    core::Graph g;
    auto *literal = g.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    auto *op = g.AddNode<core::OperatorNode>(core::NodeBase::NodeKind::kOperator);

    literal->set_type(core::NodeBase::PinDataType::kBool);
    literal->set_data(true);
    op->set_operator_type(core::OperatorNode::OperatorType::kLogicalNot);

    g.Link(literal, 0, op, 0);

    editor::code_generation::CodegenContext ctx;
    auto content = ctx.Generate(g).GetFormatedContent();
    EXPECT_NE(content.find("const bool lit_" + std::to_string(literal->id())), std::string::npos);
    EXPECT_NE(content.find("const bool tmp_" + std::to_string(op->id())), std::string::npos);
    EXPECT_NE(content.find("= false;"), std::string::npos);
}
