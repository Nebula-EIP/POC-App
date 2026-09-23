/**
 * @file nodes_list.cpp
 * @brief Contains the constructor which builds the capa containing all nodes
 * from C module
 *
 * @author Created by JeanBizeul
 * @date Created on 14-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 20-09-2026
 */

#include "modules_inits.hpp"

core::capa::NodeListCapability *CreateNodeListCapa() {
    auto *nodes = new core::capa::NodeListCapability();
    nodes->RegisterNode("Declaration", "Declares a named C value", {});
    nodes->RegisterNode("Literal", "Produces a numeric literal", {});
    nodes->RegisterNode("Binary expression", "Combines two values", {});
    nodes->RegisterNode("Conditional", "Branches on a condition", {});
    nodes->RegisterNode("Loop", "Repeats while a condition is true", {});
    return nodes;
}
