/**
 * @file node_clipboard.hpp
 * @brief Copy / paste of canvas nodes (Ctrl+C / Ctrl+V)
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "graph/graph.hpp"
#include "utils/raylib_wrapper.hpp"

namespace editor::ui {

/**
 * @brief A node with what the canvas shows of it.
 */
struct PlacedNode {
    core::NodeId id_;
    std::string title_;
    utils::WrappedVector2 position_;  ///< Top-left corner, in world space.
};

/**
 * @brief Holds copies of nodes, to paste them later, any number of times.
 *
 * Copy() snapshots the nodes into a graph of its own: the clipboard keeps
 * working after the originals are edited or deleted. Paste() creates new
 * nodes with the same type, pins and properties and no connection (like a
 * duplication), keeping their layout relative to each other.
 */
class NodeClipboard final {
   public:
    NodeClipboard();

    /**
     * @brief Replaces the clipboard content with copies of the given nodes.
     * An empty list leaves the clipboard unchanged.
     *
     * @param graph The graph holding the nodes.
     * @param nodes The nodes to copy, with their title and position. Nodes
     * missing from graph are skipped.
     */
    void Copy(const core::Graph &graph, const std::vector<PlacedNode> &nodes);

    /**
     * @brief Creates copies of the clipboard nodes in a graph.
     *
     * @param graph The graph to paste into.
     * @param anchor World position of the top-left corner of the pasted
     * group (the copied nodes' bounding box).
     *
     * @return The created nodes, with their title and position.
     */
    std::vector<PlacedNode> Paste(core::Graph &graph,
                                  utils::WrappedVector2 anchor) const;

    [[nodiscard]] bool Empty() const noexcept { return items_.empty(); }

    [[nodiscard]] std::size_t Size() const noexcept { return items_.size(); }

   private:
    struct Item {
        core::NodeId node_;  ///< Id in storage_.
        std::string title_;
        utils::WrappedVector2 offset_;  ///< From the group's top-left corner.
    };

    // Graph has no move assignment: it is replaced through the pointer.
    std::unique_ptr<core::Graph> storage_;
    std::vector<Item> items_;
};

}  // namespace editor::ui
