/**
 * @file nodes_list.cpp
 * @brief Contains the constructor which builds the capa containing all nodes from C module
 *
 * @author Created by JeanBizeul
 * @date Created on 14-09-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 14-09-2026
 */

#include "modules_inits.hpp"

core::capa::NodeListCapability *CreateNodeListCapa()
{
    return new core::capa::NodeListCapability();
}
