/**
 * @file module_inits.hpp
 * @brief Contains factories to allow loading of the module's capabilities
 *
 * @author Created by JeanBizeul
 * @date Created on 14-09-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 21-09-2026
 */

#pragma once

#include "modules/capabilities/exporter_capability.hpp"
#include "modules/capabilities/importer_capability.hpp"
#include "modules/capabilities/node_list_capability.hpp"
#include "modules/capabilities/renderer_capability.hpp"
#include "modules/capabilities/type_list_capability.hpp"

core::capa::TypeListCapability *CreateTypeListCapa();

core::capa::INodeListCapability *CreateNodeListCapa(core::capa::ITypeListCapability *types_capa);

core::capa::RendererCapability *CreateRendererCapa();
