/**
 * @file node_catalog.cpp
 * @brief Implementation of the node catalog.
 *
 * @author Created by mathys-f
 * @date Created on 25-09-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 25-09-2026
 */

#include "node_catalog.hpp"

#include "modules/module.hpp"

namespace core {

void NodeCatalog::Update(ModuleLoader &loader) {
    // 1. Assign IDs to any pending nodes in all loaded modules
    for (const IModule *module : loader.Modules()) {
        if (!module) continue;
        auto *nodes_cap = const_cast<IModule *>(module)->Nodes();
        if (!nodes_cap) continue;

        while (const std::string_view *name_ptr =
                   nodes_cap->RegisterNode(next_node_id_)) {
            std::string name_copy(
                *name_ptr);  // Force copy as required by documentation
            (void)name_copy;
            next_node_id_++;
        }
    }

    // 2. Rebuild the catalog from scratch to purge unloaded modules
    nodes_.clear();

    for (const IModule *module : loader.Modules()) {
        if (!module) continue;
        auto *nodes_cap = const_cast<IModule *>(module)->Nodes();
        if (!nodes_cap) continue;

        auto available_nodes = nodes_cap->GetAvailableNodes();
        for (const auto &meta : available_nodes) {
            NodeDescriptor desc;
            desc.id_ = meta.type_;
            desc.name_ = meta.name_;
            desc.category_ = meta.description_;
            desc.module_id_ = module->Id();

            auto config = nodes_cap->GetNodeConfiguration(meta.type_);

            for (const auto &pin : config.input_pins_) {
                desc.pins_.push_back({pin.name_, pin.type_, true});
            }
            for (const auto &pin : config.output_pins_) {
                desc.pins_.push_back({pin.name_, pin.type_, false});
            }

            nodes_.push_back(std::move(desc));
        }
    }
}

std::vector<NodeDescriptor> NodeCatalog::GetNodes() const { return nodes_; }

std::optional<NodeDescriptor> NodeCatalog::GetNode(NodeType id) const {
    for (const auto &node : nodes_) {
        if (node.id_ == id) {
            return node;
        }
    }
    return std::nullopt;
}

std::vector<NodeDescriptor> NodeCatalog::GetNodesByModule(
    ModuleId module_id) const {
    std::vector<NodeDescriptor> result;
    for (const auto &node : nodes_) {
        if (node.module_id_ == module_id) {
            result.push_back(node);
        }
    }
    return result;
}

}  // namespace core
