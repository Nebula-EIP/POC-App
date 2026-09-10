/**
 * @file node.hpp
 * @brief Implementation of the node class
 *
 * @author Created by JeanBizeul
 * @date Created on 10-09-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 11-09-2026
 */

#include <gtest/gtest.h>

#include "graph/graph.hpp"

TEST(GraphTest, CreateDeleteNode)
{
    core::Graph graph;

    core::Node &id = graph.CreateNode(1);
}
