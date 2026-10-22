/**
 * @file create_module.cpp
 * @brief Hook used to retreive the C module
 *
 * @author Created by Nolan Papa
 * @date Created on 20-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 11-10-2026
 */

#include "../modules_inits.hpp"
#include "renderer.hpp"

core::capa::RendererCapability *CreateRendererCapa() {
    auto *renderer = new core::capa::RendererCapability();
    // Component callbacks report their changes to the application's handler.
    c_module::renderer::RegisterProviders(
        *renderer,
        [renderer](core::NodeId node_id, const core::PropertyMap &changes) {
            renderer->NotifyPropertyChange(node_id, changes);
        });
    return renderer;
}
