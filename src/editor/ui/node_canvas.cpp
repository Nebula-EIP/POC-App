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
    views_[node.Id()] =
        std::make_unique<NodeView>(node, std::move(title), position);
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
    draw_order_.erase(
        std::remove_if(draw_order_.begin(), draw_order_.end(),
                       [&](core::NodeId id) { return !views_.contains(id); }),
        draw_order_.end());
}

std::optional<HitResult> NodeCanvas::HitTest(
    utils::WrappedVector2 point) const {
    for (auto it = draw_order_.rbegin(); it != draw_order_.rend(); ++it) {
        const auto kHit = views_.at(*it)->HitTest(point);
        if (kHit.part_ != HitPart::kNone) return kHit;
    }
    return std::nullopt;
}

void NodeCanvas::SelectOnly(core::NodeId node_id) {
    for (auto &[id, view] : views_) view->SetSelected(id == node_id);
    const auto kPosition =
        std::find(draw_order_.begin(), draw_order_.end(), node_id);
    if (kPosition != draw_order_.end()) {
        draw_order_.erase(kPosition);
        draw_order_.push_back(node_id);
    }
}

void NodeCanvas::SelectInRectangle(utils::WrappedRectangle rectangle) {
    for (auto &[id, view] : views_) {
        (void)id;
        view->SetSelected(
            utils::CheckCollisionRecsWrapped(rectangle, view->Bounds()));
    }
}

void NodeCanvas::ProcessInput() {
    const auto kCursor =
        camera_.ScreenToWorld(utils::GetCursorPositionWrapped());
    if (utils::IsLeftClicked()) {
        const auto kHit = HitTest(kCursor);
        const bool kAdditive =
            utils::IsKeyDownWrapped(utils::WrappedKey::kLeftControl);
        if (kHit.has_value()) {
            if (kAdditive) {
                auto &view = *views_.at(kHit->node_id_);
                view.SetSelected(!view.Selected());
            } else if (!views_.at(kHit->node_id_)->Selected()) {
                SelectOnly(kHit->node_id_);
            }
            drag_start_ = kCursor;
            drag_origins_.clear();
            for (const auto &[id, view] : views_) {
                if (view->Selected()) {
                    drag_origins_[id] = {view->Bounds().x_, view->Bounds().y_};
                }
            }
        } else {
            if (!kAdditive) {
                for (auto &[id, view] : views_) {
                    (void)id;
                    view->SetSelected(false);
                }
            }
            selection_start_ = kCursor;
        }
    }
    if (utils::IsLeftDown() && drag_start_.has_value()) {
        const auto kDelta = utils::WrappedVector2{kCursor.x_ - drag_start_->x_,
                                                  kCursor.y_ - drag_start_->y_};
        for (const auto &[id, origin] : drag_origins_) {
            auto &view = *views_.at(id);
            const auto kCurrent =
                utils::WrappedVector2{view.Bounds().x_, view.Bounds().y_};
            view.MoveBy({origin.x_ + kDelta.x_ - kCurrent.x_,
                         origin.y_ + kDelta.y_ - kCurrent.y_});
        }
    }
    if (utils::IsLeftDown() && selection_start_.has_value()) {
        SelectInRectangle(SelectionRectangle(*selection_start_, kCursor));
    }
    if (!utils::IsLeftDown()) {
        drag_start_.reset();
        selection_start_.reset();
        drag_origins_.clear();
    }
}

void NodeCanvas::Draw() const {
    for (const auto kId : draw_order_) views_.at(kId)->Draw(camera_);
}

}  // namespace editor::ui
