/**
 * @file node_catalog.hpp
 * @brief Facade for accessing module nodes cleanly for the renderer.
 *
 * @author Created by mathys-f
 * @date Created on 25-09-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 25-09-2026
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "graph/datatypes.hpp"
#include "modules/loader.hpp"

namespace core {

/**
 * @brief Describes a pin of a node.
 */
struct NodePinDescriptor {
    std::string name_;
    DataType type_;
    bool is_input_;
};

/**
 * @brief Describes a node provided by a module.
 */
struct NodeDescriptor {
    NodeType id_;
    std::string name_;
    std::string category_;
    std::vector<NodePinDescriptor> pins_;
    ModuleId module_id_;
};

/**
 * @brief Facade over the ModuleLoader to retrieve node definitions.
 * Designed to provide the renderer with node data without exposing
 * capabilities or the module loader directly.
 */
class NodeCatalog {
   public:
    NodeCatalog() = default;
    ~NodeCatalog() = default;

    /**
     * @brief Refreshes the catalog by reading the available nodes from the
     * loader. Must be called after modules are loaded or unloaded.
     * @param loader The module loader containing the loaded modules.
     */
    void Update(ModuleLoader &loader);

    /**
     * @brief Gets all nodes currently available in the catalog.
     * @return A vector of node descriptors.
     */
    std::vector<NodeDescriptor> GetNodes() const;

    /**
     * @brief Gets a specific node by its ID.
     * @param id The node type ID to search for.
     * @return An optional containing the node descriptor if found.
     */
    std::optional<NodeDescriptor> GetNode(NodeType id) const;

    /**
     * @brief Gets all nodes provided by a specific module.
     * @param module_id The module ID to filter by.
     * @return A vector of node descriptors provided by the given module.
     */
    std::vector<NodeDescriptor> GetNodesByModule(ModuleId module_id) const;

   private:
    NodeType next_node_id_ = 1;
    std::vector<NodeDescriptor> nodes_;
};

}  // namespace core
