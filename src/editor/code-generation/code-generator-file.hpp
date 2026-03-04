#pragma once
#include <string>
#include <vector>

#include "code-generation/code-generator-errors.hpp"

namespace code_generation {
class CodeGeneratorFile {
 public:
    CodeGeneratorFile() = default;

    /**
     * @brief Adds a line of code to the file content at the current cursor position.
     * @param line The line of code to be added.
     * @return New cursor position
     */
    int Line(const std::string& line);

    /**
     * @brief Adds a line of code to the file content at a specified position.
     * @param line The line of code to be added.
     * @param position The position at which to add the line of code. If the position is out of bounds, it will be wrapped around.
     * @return New cursor position
     */
    int LineAt(const std::string& line, int position);

    /**
     * @brief Opens a new code block with the specified header (e.g., "if (condition) {") and adds it to the content.
     *        The block will be closed with a corresponding closing brace "}" when CloseBlock() is called.
     * @param block_header The header of the code block to be opened.
     * @return True if the block was successfully opened, false otherwise (e.g., if the block header is invalid).
     */
    bool OpenBlock(const std::string& block_header);

    /**
     * @brief Closes the most recently opened code block by adding a closing brace "}" to the content.
     * @return True if a block was successfully closed, false otherwise (e.g., if there are no open blocks to close).
     */
    bool CloseBlock();

    /**
     * @brief Closes the most recently opened code block and immediately opens a new block with the specified header.
     * @param block_header The header of the new code block to be opened after closing the current block.
     * @return True if the current block was successfully closed and the new block was successfully opened, false otherwise (e.g., if there are no open blocks to close or if the new block header is invalid).
     */
    bool CloseAndOpenBlock(const std::string& block_header);

    /**
     * @brief Closes all currently open code blocks by adding the appropriate number of closing braces "}" to the content.
     * @return True if all blocks were successfully closed, false otherwise (e.g., if there are no open blocks to close).
     */
    bool CloseAllBlocks();

    /**
     * @brief Sets the indentation level for the formatted content.
     * @param level The indentation level to be set. Each level corresponds to a fixed number of spaces.
     */
    void SetIndentLevel(int level);

    /**
     * @brief Sets the cursor position for adding new lines of code.
     * @param position The new cursor position to be set.
     * @return The new cursor position after setting it.
     */
    int SetCursor(int position);

    /**
     * @brief Retrieves the position of the most recently opened code block, which can be used to determine where to insert new lines of code within that block.
     * @return The position of the most recently opened code block, or -1 if there
     */
    int GetPositionStartBlock() const;

    /**
     * @brief Moves the cursor by a specified offset from its current position.
     * @param offset The number of positions to move the cursor. Positive values move it forward
     *               and negative values move it backward.
     * @return The new cursor position after moving it.
     */
    int MoveCursor(int offset);

    /**
     * @brief Retrieves the current content of the code file.
     * @return A string representing the current content of the code file.
     */
    std::string GetContent() const;

    /**
     * @brief Retrieves the formatted content of the code file, applying indentation, carriage returns, and other formatting rules as necessary.
     * @return A string representing the formatted content of the code file with proper indentation.
     */
    std::string GetFormatedContent() const;

 private:
    /**
     * @brief Return the position contained in the content, accounting for wrapping around if the position is out of bounds.
     * @param position The position to be contained within the content.
     * @return The contained position within the content.
    */
    int GetContainedPosition(int position) const;

    std::vector<std::string> content_;
    std::vector<int> block_positions_;
    int cursor_ = 0;
    int indent_level_ = 4;
};
}  // namespace code_generation