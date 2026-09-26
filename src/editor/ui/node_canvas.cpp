/**
 * @file application.cpp
 * @brief Implementation of the Application class, which manages the main loop,
 * window, and module loading for the editor.
 *
 * @author Created by Nolan Papa
 * @date Created on 26-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 26-09-2026
 */
#include "node_canvas.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

namespace editor::ui {
namespace {
utils::WrappedRectangle SelectionRectangle(utils::WrappedVector2 first,
                                           utils::WrappedVector2 second) {
    return {std::min(first.x_, second.x_), std::min(first.y_, second.y_),
            std::abs(first.x_ - second.x_), std::abs(first.y_ - second.y_)};
}
}  // namespace

void NodeCanvas::AddNode(const core::Node &node, std::string title,
                         utils::WrappedVector2 position) {
    views_[node.Id()] = std::make_unique<NodeView>(node, std::move(title), position);
    if (std::find(draw_order_.begin(), draw_order_.end(), node.Id()) ==
        draw_order_.end()) {
        draw_order_.push_back(node.Id());
    }
}

void NodeCanvas::RemoveMissingNodes(const core::Graph &graph) {
    for (auto it = views_.begin(); it != views_.end();) {
        if (!graph.HasNode(it->first)) {
            it = views_.erase(it);
        } else {
            ++it;
        }
    }
    draw_order_.erase(std::remove_if(draw_order_.begin(), draw_order_.end(),
                                     [&](core::NodeId id) {
                                         return !views_.contains(id);
                                     }),
                      draw_order_.end());
}

std::optional<HitResult> NodeCanvas::HitTest(
    utils::WrappedVector2 point) const {
    for (auto it = draw_order_.rbegin(); it != draw_order_.rend(); ++it) {
        const auto hit = views_.at(*it)->HitTest(point);
        if (hit.part_ != HitPart::kNone) return hit;
    }
    return std::nullopt;
}

void NodeCanvas::SelectOnly(core::NodeId node_id) {
    for (auto &[id, view] : views_) view->SetSelected(id == node_id);
    const auto position = std::find(draw_order_.begin(), draw_order_.end(), node_id);
    if (position != draw_order_.end()) {
        draw_order_.erase(position);
        draw_order_.push_back(node_id);
    }
}

void NodeCanvas::SelectInRectangle(utils::WrappedRectangle rectangle) {
    for (auto &[id, view] : views_) {
        (void)id;
        view->SetSelected(utils::CheckCollisionRecsWrapped(
            rectangle, view->Bounds()));
    }
}

void NodeCanvas::ProcessInput() {
    const auto cursor = camera_.ScreenToWorld(utils::GetCursorPositionWrapped());
    if (utils::IsLeftClicked()) {
        const auto hit = HitTest(cursor);
        const bool additive =
            utils::IsKeyDownWrapped(utils::WrappedKey::kLeftControl);
        if (hit.has_value()) {
            if (additive) {
                auto &view = *views_.at(hit->node_id_);
                view.SetSelected(!view.Selected());
            } else if (!views_.at(hit->node_id_)->Selected()) {
                SelectOnly(hit->node_id_);
            }
            drag_start_ = cursor;
            drag_origins_.clear();
            for (const auto &[id, view] : views_) {
                if (view->Selected()) {
                    drag_origins_[id] = {view->Bounds().x_, view->Bounds().y_};
                }
            }
        } else {
            if (!additive) {
                for (auto &[id, view] : views_) {
                    (void)id;
                    view->SetSelected(false);
                }
            }
            selection_start_ = cursor;
        }
    }
    if (utils::IsLeftDown() && drag_start_.has_value()) {
        const auto delta = utils::WrappedVector2{cursor.x_ - drag_start_->x_,
                                                 cursor.y_ - drag_start_->y_};
        for (const auto &[id, origin] : drag_origins_) {
            auto &view = *views_.at(id);
            const auto current = utils::WrappedVector2{view.Bounds().x_,
                                                       view.Bounds().y_};
            view.MoveBy({origin.x_ + delta.x_ - current.x_,
                         origin.y_ + delta.y_ - current.y_});
        }
    }
    if (utils::IsLeftDown() && selection_start_.has_value()) {
        SelectInRectangle(SelectionRectangle(*selection_start_, cursor));
    }
    if (!utils::IsLeftDown()) {
        drag_start_.reset();
        selection_start_.reset();
        drag_origins_.clear();
    }
}

void NodeCanvas::Draw() const {
    for (const auto id : draw_order_) views_.at(id)->Draw(camera_);
}

}  // namespace editor::ui
