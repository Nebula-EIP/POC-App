/**
 * @file create_module.cpp
 * @brief Hook used to retreive the C module
 *
 * @author Created by Nolan Papa
 * @date Created on 20-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 20-09-2026
 */

#include "../modules_inits.hpp"
#include "renderer.hpp"

core::capa::RendererCapability *CreateRendererCapa() {
    auto *renderer = new core::capa::RendererCapability();
    c_module::renderer::RegisterProviders(*renderer);
    return renderer;
}
