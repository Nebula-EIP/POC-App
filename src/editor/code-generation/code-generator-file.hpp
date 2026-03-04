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
    int AddContent(const std::string& line);

    /**
     * @brief Adds a line of code to the file content at a specified position.
     * @param line The line of code to be added.
     * @param position The position in the content where the line should be inserted.
     * @return New cursor position
     */
    int AddContentAt(const std::string& line, int position);

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

    std::string content_ = "";
    int cursor_ = 0;
    int indent_level_ = 4;
};
}  // namespace code_generation