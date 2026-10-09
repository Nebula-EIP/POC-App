/**
 * @file node_canvas.cpp
 * @brief Implementation of the NodeCanvas class, which manages the display and
 * interaction of nodes in the editor.
 *
 * @author Created by Nolan Papa
 * @date Created on 26-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 09-10-2026
 */
#include "node_canvas.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

namespace editor::ui {
namespace {
constexpr utils::WrappedColor kSelectionFill = {100, 160, 255, 40};
constexpr utils::WrappedColor kSelectionBorder = {100, 160, 255, 200};

utils::WrappedRectangle MakeRectangle(utils::WrappedVector2 first,
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

    std::erase_if(drag_origins_, [&](const auto &entry) {
        return !views_.contains(entry.first);
    });
    if (drag_origins_.empty()) {
        drag_start_.reset();
    }
    if (connection_drag_start_ &&
        !views_.contains(connection_drag_start_->node_id_)) {
        connection_drag_start_.reset();
        connection_drag_current_.reset();
    }
    if (pending_connection_ &&
        (!views_.contains(pending_connection_->first.node_id_) ||
         !views_.contains(pending_connection_->second.node_id_))) {
        pending_connection_.reset();
    }
}

std::vector<core::NodeId> NodeCanvas::SelectedNodes() const {
    std::vector<core::NodeId> selected;
    for (const auto kId : draw_order_) {
        if (views_.at(kId)->Selected()) {
            selected.push_back(kId);
        }
    }
    return selected;
}

void NodeCanvas::SetSelection(const std::vector<core::NodeId> &node_ids) {
    for (auto &[id, view] : views_) {
        view->SetSelected(std::find(node_ids.begin(), node_ids.end(), id) !=
                          node_ids.end());
    }
    for (const auto kId : node_ids) {
        const auto kPosition =
            std::find(draw_order_.begin(), draw_order_.end(), kId);
        if (kPosition != draw_order_.end()) {
            draw_order_.erase(kPosition);
            draw_order_.push_back(kId);
        }
    }
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
        const bool kInBase =
            std::find(selection_base_.begin(), selection_base_.end(), id) !=
            selection_base_.end();
        view->SetSelected(kInBase || utils::CheckCollisionRecsWrapped(
                                         rectangle, view->Bounds()));
    }
}

void NodeCanvas::ProcessInput(const render::Camera &camera) {
    const auto kCursor =
        camera.ScreenToWorld(utils::GetCursorPositionWrapped());

    if (connection_drag_start_ && utils::IsLeftDown()) {
        connection_drag_current_ = kCursor;
    }

    if (utils::IsLeftClicked()) {
        const auto kHit = HitTest(kCursor);
        const bool kAdditive =
            utils::IsKeyDownWrapped(utils::WrappedKey::kLeftControl);
        if (kHit.has_value()) {
            if (kHit->part_ == HitPart::kInputPin ||
                kHit->part_ == HitPart::kOutputPin) {
                connection_drag_start_ = kHit;
                connection_drag_current_ = kCursor;
            } else {
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
                        drag_origins_[id] = {view->Bounds().x_,
                                             view->Bounds().y_};
                    }
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
            selection_current_ = kCursor;
            // With Ctrl, the rectangle adds to the current selection.
            selection_base_ =
                kAdditive ? SelectedNodes() : std::vector<core::NodeId>{};
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
        selection_current_ = kCursor;
        SelectInRectangle(MakeRectangle(*selection_start_, kCursor));
    }
    if (!utils::IsLeftDown()) {
        if (connection_drag_start_) {
            const auto kHit = HitTest(kCursor);
            if (kHit.has_value() && (kHit->part_ == HitPart::kInputPin ||
                                     kHit->part_ == HitPart::kOutputPin)) {
                pending_connection_ =
                    std::make_pair(*connection_drag_start_, *kHit);
            }
            connection_drag_start_.reset();
            connection_drag_current_.reset();
        }
        drag_start_.reset();
        selection_start_.reset();
        selection_current_.reset();
        selection_base_.clear();
        drag_origins_.clear();
    }
}

void NodeCanvas::Draw(const render::Camera &camera) const {
    for (const auto kId : draw_order_) views_.at(kId)->Draw(camera);
    DrawSelectionRectangle(camera);
}

std::optional<utils::WrappedRectangle> NodeCanvas::ActiveSelectionRectangle()
    const noexcept {
    if (!selection_start_ || !selection_current_) {
        return std::nullopt;
    }
    return MakeRectangle(*selection_start_, *selection_current_);
}

void NodeCanvas::DrawSelectionRectangle(const render::Camera &camera) const {
    const auto kRectangle = ActiveSelectionRectangle();
    if (!kRectangle) {
        return;
    }

    utils::DrawRectangleRecWrapped(*kRectangle, kSelectionFill);

    const float kThickness = 1.0F / camera.Zoom();
    const float kLeft = kRectangle->x_;
    const float kTop = kRectangle->y_;
    const float kRight = kRectangle->x_ + kRectangle->width_;
    const float kBottom = kRectangle->y_ + kRectangle->height_;
    utils::DrawLineWrapped({kLeft, kTop}, {kRight, kTop}, kThickness,
                           kSelectionBorder);
    utils::DrawLineWrapped({kRight, kTop}, {kRight, kBottom}, kThickness,
                           kSelectionBorder);
    utils::DrawLineWrapped({kRight, kBottom}, {kLeft, kBottom}, kThickness,
                           kSelectionBorder);
    utils::DrawLineWrapped({kLeft, kBottom}, {kLeft, kTop}, kThickness,
                           kSelectionBorder);
}

}  // namespace editor::ui