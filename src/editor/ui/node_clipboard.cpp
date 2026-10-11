/**
 * @file node_clipboard.cpp
 * @brief Implementation of the node clipboard
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 */

#include "node_clipboard.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace editor::ui {

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
    for (const PlacedNode &node : nodes) {
        if (!graph.HasNode(node.id_)) {
            continue;
        }
        items.push_back(Item{
            .node_ = storage->CopyNodeFrom(graph, node.id_).Id(),
            .title_ = node.title_,
            .offset_ = {node.position_.x_ - top_left.x_,
                        node.position_.y_ - top_left.y_},
        });
    }
    storage_ = std::move(storage);
    items_ = std::move(items);
}

std::vector<PlacedNode> NodeClipboard::Paste(
    core::Graph &graph, utils::WrappedVector2 anchor) const {
    std::vector<PlacedNode> pasted;
    pasted.reserve(items_.size());
    for (const Item &item : items_) {
        pasted.push_back(PlacedNode{
            .id_ = graph.CopyNodeFrom(*storage_, item.node_).Id(),
            .title_ = item.title_,
            .position_ = {anchor.x_ + item.offset_.x_,
                          anchor.y_ + item.offset_.y_},
        });
    }
    return pasted;
}

}  // namespace editor::ui
