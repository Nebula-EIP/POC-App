/**
 * @file nodes_list.cpp
 * @brief Contains the constructor which builds the capa containing all nodes from C module
 *
 * @author Created by JeanBizeul
 * @date Created on 14-09-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 21-09-2026
 */

#include "modules_inits.hpp"
#include "nodes/c_node_list_capability.hpp"

core::capa::INodeListCapability *CreateNodeListCapa(core::capa::ITypeListCapability *types_capa)
{
    return new c_module::CNodeListCapability(types_capa);
}
