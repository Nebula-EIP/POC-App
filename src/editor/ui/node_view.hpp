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

#pragma once

#include <string>
#include <vector>

#include "graph/node.hpp"
#include "utils/raylib_wrapper.hpp"

namespace editor::ui {

struct Camera {
    utils::WrappedVector2 offset_{0.0F, 0.0F};
    float zoom_ = 1.0F;

    utils::WrappedVector2 ScreenToWorld(utils::WrappedVector2 point) const
        noexcept;
    utils::WrappedVector2 WorldToScreen(utils::WrappedVector2 point) const
        noexcept;
};

enum class HitPart { kNone, kBody, kInputPin, kOutputPin };

struct HitResult {
    core::NodeId node_id_ = 0;
    core::PinId pin_id_ = 0;
    HitPart part_ = HitPart::kNone;
};

class NodeView final {
   public:
    NodeView(const core::Node &node, std::string title,
             utils::WrappedVector2 position);

    core::NodeId NodeId() const noexcept { return node_id_; }
    const utils::WrappedRectangle &Bounds() const noexcept { return bounds_; }
    const std::string &Title() const noexcept { return title_; }
    bool Selected() const noexcept { return selected_; }
    void SetSelected(bool selected) noexcept { selected_ = selected; }
    void MoveBy(utils::WrappedVector2 delta) noexcept;

    HitResult HitTest(utils::WrappedVector2 world_point) const noexcept;
    void Draw(const Camera &camera) const;

   private:
    struct PortView {
        core::PinId id_;
        std::string name_;
        core::DataType type_;
        utils::WrappedRectangle bounds_;
    };

    static utils::WrappedColor TypeColor(core::DataType type) noexcept;
    static float TextWidth(const std::string &text) noexcept;
    void RebuildLayout(const core::Node &node);

    core::NodeId node_id_;
    core::NodeType node_type_;
    std::string title_;
    utils::WrappedRectangle bounds_{};
    std::vector<PortView> inputs_;
    std::vector<PortView> outputs_;
    bool selected_ = false;
};

}  // namespace editor::ui
