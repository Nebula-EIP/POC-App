#include <iostream>

#include "connection_exceptions.hpp"
#include "exception_base.hpp"
#include "graph.hpp"
#include "logger.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/variable_node.hpp"

static void DumpGraph(const core::Graph &graph) {
    printf("### DUMPING GRAPH ###\n");
    for (const auto &node : graph.GetAllNodes()) {
        printf("Node[%d:%s]\n", node->id(),
               core::NodeKindToString(node->kind()).c_str());

        printf("  Inputs: %ld/%d\n", node->GetAllParents().size(),
               node->GetInputPinCount());
        for (auto conn : node->GetAllParents()) {
            if (conn.IsConnected()) {
                printf("    IN[%d:%s] <- Node[%d:%s] Out[%d:%s]\n", conn.in_pin,
                       core::PinDataTypeToString(conn.type).c_str(),
                       conn.node->id(), node->GetDisplayName().c_str(),
                       conn.out_pin,
                       core::PinDataTypeToString(conn.type).c_str());
            } else {
                printf("    IN[%d:%s] <- Disconnected\n", conn.in_pin,
                       core::PinDataTypeToString(conn.type).c_str());
            }
        }

        printf("  Outputs: %ld/%d\n", node->GetAllChildrens().size(),
               node->GetOutputPinCount());
        for (auto conn : node->GetAllChildrens()) {
            if (conn.IsConnected()) {
                printf(
                    "    OUT[%d:%s] -> Node[%d:%s] In[%d:%s]\n", conn.out_pin,
                    core::PinDataTypeToString(conn.type).c_str(),
                    conn.node->id(),
                    core::NodeKindToString(conn.node->kind()).c_str(),
                    conn.in_pin, core::PinDataTypeToString(conn.type).c_str());
            } else {
                printf("    OUT[%d:%s] -> Disconnected\n", conn.out_pin,
                       core::PinDataTypeToString(conn.type).c_str());
            }
        }
    }
    printf("### COMPLETED ###\n");
}

int main(void) {
    try {
        core::Graph graph;

        auto lit1 = graph.AddNode<core::LiteralNode>(
            core::NodeBase::NodeKind::kLiteral, {0, 0});
        auto lit2 = graph.AddNode<core::LiteralNode>(
            core::NodeBase::NodeKind::kLiteral, {0, 0});
        auto var1 = graph.AddNode<core::VariableNode>(
            core::NodeBase::NodeKind::kVariable, {0, 0});

        DumpGraph(graph);
        graph.Link(lit1, 0, var1, 0);
        DumpGraph(graph);
        graph.Unlink(lit1, 0, var1, 0);
        DumpGraph(graph);
    } catch (const utils::BaseException &e) {
        LOG_FATAL(e.GetDetailedMessage());
    }

    return 0;
}
