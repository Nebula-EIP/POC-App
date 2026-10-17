/**
 * @file node_clipboard.cpp
 * @brief Implementation of the node clipboard
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 12-10-2026
 */

#include "node_clipboard.hpp"

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <utility>

namespace editor::ui {

namespace {

/**
 * @brief Recreates in target the connections of source whose two ends were
 * copied. Pin ids are kept by Graph::CopyNodeFrom, only node ids change.
 *
 * @param copies Node id in source -> node id of its copy in target.
 */
void CopyConnections(
    const core::Graph &source, core::Graph &target,
    const std::unordered_map<core::NodeId, core::NodeId> &copies) {
    for (const auto &[id, connection] : source.GetAllConnections()) {
        (void)id;
        const auto kFrom = copies.find(connection.from_node_);
        const auto kTo = copies.find(connection.to_node_);
        if (kFrom == copies.end() || kTo == copies.end()) {
            continue;  // One end was not copied.
        }
        target.Connect(kFrom->second, connection.out_pin_, kTo->second,
                       connection.in_pin_);
    }
}

}  // namespace

NodeClipboard::NodeClipboard() : storage_(std::make_unique<core::Graph>()) {}

void NodeClipboard::Copy(const core::Graph &graph,
                         const std::vector<PlacedNode> &nodes) {
    utils::WrappedVector2 top_left{std::numeric_limits<float>::max(),
                                   std::numeric_limits<float>::max()};
    bool any = false;
    for (const PlacedNode &node : nodes) {
        if (graph.HasNode(node.id_)) {
            top_left.x_ = std::min(top_left.x_, node.position_.x_);
            top_left.y_ = std::min(top_left.y_, node.position_.y_);
            any = true;
        }
    }
    if (!any) {
        return;
    }

    auto storage = std::make_unique<core::Graph>();
    std::vector<Item> items;
    items.reserve(nodes.size());
    std::unordered_map<core::NodeId, core::NodeId> copies;
    for (const PlacedNode &node : nodes) {
        // Skips missing nodes, and nodes listed twice.
        if (!graph.HasNode(node.id_) || copies.contains(node.id_)) {
            continue;
        }
        const core::NodeId kCopy = storage->CopyNodeFrom(graph, node.id_).Id();
        copies.emplace(node.id_, kCopy);
        items.push_back(Item{
            .node_ = kCopy,
            .title_ = node.title_,
            .offset_ = {node.position_.x_ - top_left.x_,
                        node.position_.y_ - top_left.y_},
        });
    }
    CopyConnections(graph, *storage, copies);
    storage_ = std::move(storage);
    items_ = std::move(items);
}

std::vector<PlacedNode> NodeClipboard::Paste(
    core::Graph &graph, utils::WrappedVector2 anchor) const {
    std::vector<PlacedNode> pasted;
    pasted.reserve(items_.size());
    std::unordered_map<core::NodeId, core::NodeId> copies;
    for (const Item &item : items_) {
        const core::NodeId kCopy =
            graph.CopyNodeFrom(*storage_, item.node_).Id();
        copies.emplace(item.node_, kCopy);
        pasted.push_back(PlacedNode{
            .id_ = kCopy,
            .title_ = item.title_,
            .position_ = {anchor.x_ + item.offset_.x_,
                          anchor.y_ + item.offset_.y_},
        });
    }
    CopyConnections(*storage_, graph, copies);
    return pasted;
}

}  // namespace editor::ui
