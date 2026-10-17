/**
 * @file node_picker.hpp
 * @brief Declaration of the NodePicker class, the searchable tree of the nodes
 * provided by the loaded modules, used to add a node to the canvas.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 11-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 12-10-2026
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "graph/datatypes.hpp"
#include "modules/loader.hpp"
#include "node_catalog/node_catalog.hpp"
#include "top_bar.hpp"
#include "utils/raylib_wrapper.hpp"

namespace editor::ui {

/**
 * @brief One node the picker can create.
 */
struct NodePickerEntry {
    core::NodeType type_ = 0;
    std::string name_;
    std::string module_;    ///< Top-level folder of the entry.
    std::string category_;  ///< Sub-folders, separated by '/'. May be empty.
};

/**
 * @brief Builds the picker entries from the node catalog, in catalog order.
 *
 * @param catalog The catalog of the nodes of the loaded modules.
 * @param module_name Returns the displayed name of a module.
 */
std::vector<NodePickerEntry> EntriesFromCatalog(
    const core::NodeCatalog &catalog,
    const std::function<std::string(core::ModuleId)> &module_name);

/**
 * @brief Mouse and keyboard state the picker reacts to, for one frame.
 *
 * Kept separate from raylib so that the picker can be tested without a window.
 * Use NodePicker::ReadInput() in the application.
 */
struct NodePickerInput {
    utils::WrappedVector2 cursor_{0.0F, 0.0F};
    bool left_pressed_ = false;
    bool right_pressed_ = false;
    bool escape_pressed_ = false;
    bool enter_pressed_ = false;
    bool up_pressed_ = false;
    bool down_pressed_ = false;
    bool left_key_pressed_ = false;
    bool right_key_pressed_ = false;
    bool backspace_pressed_ = false;
    std::string typed_;  ///< Characters typed this frame.
    /// The key of the shortcut that opened the picker is still held.
    bool opening_key_down_ = false;
    float wheel_ = 0.0F;  ///< Positive when the wheel is scrolled up.
    float screen_width_ = 0.0F;
    float screen_height_ = 0.0F;
};

/**
 * @brief A floating, searchable tree of the available nodes.
 *
 * Without a search, the nodes are shown as a tree: one folder per module,
 * then the category folders of the module ("Flow/Loops" gives Flow > Loops).
 * Folders start closed (a single module folder starts open). A click,
 * Enter or Right opens a folder; a click, Enter or Left closes it.
 *
 * Typing a search replaces the tree with the flat list of the nodes whose
 * name contains the text (case insensitive), without folders; names starting
 * with the text come first.
 *
 * Enter or a click on a node picks it: the picker closes and calls the
 * callback given to Open() with the node type. Escape or a click outside
 * closes it without creating anything.
 *
 * Each frame, call Update() before anything else reads the input: while it
 * is open the picker owns the mouse and the keyboard. Then call Draw() after
 * the canvas has been drawn.
 */
class NodePicker final {
   public:
    using TextMeasurer = std::function<float(const std::string &, int)>;
    using PickCallback = std::function<void(core::NodeType)>;

    /// Kind of a displayed row.
    enum class RowKind : std::uint8_t { kFolder, kEntry, kEmpty };

    /// A displayed row: a folder, a node, or the "no result" line.
    struct Row {
        RowKind kind_ = RowKind::kEntry;
        std::string label_;
        std::size_t depth_ = 0;  ///< Nesting level, 0 at the top.
        std::size_t entry_ = 0;  ///< Index in Entries(), for kEntry rows.
        std::string path_;       ///< Folder path, for kFolder rows.
        bool expanded_ = false;  ///< For kFolder rows.
    };

    /**
     * @param style Sizes and colors, shared with the top bar.
     * @param measurer Text measuring function. Defaults to raylib's, which
     * needs an open window; tests pass their own.
     */
    explicit NodePicker(TopBarStyle style = {}, TextMeasurer measurer = {});

    /**
     * @brief Replaces the nodes the picker lists and rebuilds the tree.
     * Folders and nodes keep the order of their first appearance.
     */
    void SetEntries(std::vector<NodePickerEntry> entries);

    [[nodiscard]] const std::vector<NodePickerEntry> &Entries() const noexcept {
        return entries_;
    }

    /**
     * @brief Opens the picker with an empty search and closed folders, its
     * top-left corner at a screen position, moved so that it stays on screen.
     *
     * @param on_pick Called with the type of the picked node.
     */
    void Open(utils::WrappedVector2 position, float screen_width,
              float screen_height, PickCallback on_pick);

    void Close() noexcept;

    [[nodiscard]] bool IsOpen() const noexcept { return is_open_; }

    /**
     * @brief Reads this frame's input from raylib. Consumes the typed
     * characters: call it only while the picker is open.
     */
    static NodePickerInput ReadInput();

    /**
     * @brief Handles one frame of input.
     *
     * @return true when the input belongs to the picker and must not reach
     * the rest of the editor. A click outside closes the picker and returns
     * false, so that the click still reaches what is under it.
     */
    bool Update(const NodePickerInput &input);

    void Draw() const;

    [[nodiscard]] const std::string &Query() const noexcept { return query_; }

    /// Whether a search is typed: the rows are then a flat list.
    [[nodiscard]] bool IsSearching() const noexcept { return !query_.empty(); }

    /// Whether the folder with this path ("C", "C/Flow", ...) is open.
    [[nodiscard]] bool IsExpanded(const std::string &path) const {
        return expanded_.contains(path);
    }

    /// Displayed rows, scrolled out ones included.
    [[nodiscard]] const std::vector<Row> &Rows() const noexcept {
        return rows_;
    }

    /// Row highlighted by the keyboard or the mouse, used by Enter.
    [[nodiscard]] std::optional<std::size_t> SelectedRow() const noexcept {
        return selected_;
    }

    /// Index of the first displayed row.
    [[nodiscard]] std::size_t Scroll() const noexcept { return scroll_; }

    /// Number of rows displayed at once.
    [[nodiscard]] std::size_t VisibleRowCount() const noexcept {
        return visible_rows_;
    }

    [[nodiscard]] utils::WrappedRectangle Bounds() const;

    /// Screen rectangle of a row, or nothing when it is scrolled out.
    [[nodiscard]] std::optional<utils::WrappedRectangle> RowRect(
        std::size_t row) const;

   private:
    struct Folder {
        std::string name_;
        std::string path_;
        std::vector<std::size_t> folders_;  ///< Indices in folders_.
        std::vector<std::size_t> entries_;  ///< Indices in entries_.
    };

    void BuildTree();
    std::size_t FolderChild(std::optional<std::size_t> parent,
                            const std::string &name);
    void AddFolderRows(std::size_t folder, std::size_t depth);
    void AddSearchRows();
    /// Rebuilds rows_, keeping the selection on the same folder or node when
    /// it is still displayed, else selecting the first row.
    void RebuildRows();
    [[nodiscard]] std::string RowKey(std::size_t row) const;

    void SetExpanded(std::size_t row, bool expanded);
    void Activate(std::size_t row);
    void MoveSelection(int direction);
    void SelectParent();
    void ScrollBy(int rows);
    void EnsureSelectedVisible();
    void Pick(std::size_t row);

    [[nodiscard]] std::size_t MaxScroll() const noexcept;
    [[nodiscard]] float Width() const noexcept { return width_; }
    [[nodiscard]] float ComputeWidth() const;
    [[nodiscard]] float TextX(std::size_t depth) const;
    [[nodiscard]] utils::WrappedRectangle SearchRect() const;
    [[nodiscard]] std::optional<std::size_t> HitTestRow(
        utils::WrappedVector2 point) const;

    TopBarStyle style_;
    TextMeasurer measurer_;
    std::vector<NodePickerEntry> entries_;
    std::vector<Folder> folders_;
    std::vector<std::size_t> roots_;
    std::set<std::string> expanded_;
    float width_ = 0.0F;  ///< Fits the longest row, all folders open.

    bool is_open_ = false;
    utils::WrappedVector2 position_{0.0F, 0.0F};
    PickCallback on_pick_;

    std::string query_;
    std::vector<Row> rows_;
    std::optional<std::size_t> selected_;
    std::size_t scroll_ = 0;
    std::size_t visible_rows_ = 1;
    std::optional<utils::WrappedVector2> last_cursor_;
    /// Typed characters go to the search. Off when the picker opens, so that
    /// the character of the opening shortcut is not typed.
    bool typing_armed_ = false;
    static inline bool skip_typed = false;
};

}  // namespace editor::ui
