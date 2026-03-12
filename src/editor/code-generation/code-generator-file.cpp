#include "code-generation/code-generator-file.hpp"

namespace code_generation {
size_t CodeGeneratorFile::Line(const std::string &line) {
    LineAt(line, cursor_);
    return cursor_;
}

size_t CodeGeneratorFile::LineAt(const std::string &line, int position) {
    size_t new_pos = GetContainedPosition(position);
    content_.insert(content_.begin() + new_pos, line);
    if (new_pos <= cursor_) cursor_++;

    return cursor_;
}

bool CodeGeneratorFile::OpenBlock(const std::string &block_header) {
    block_positions_.push_back(cursor_);
    Line(block_header + " {");
    return true;
}

bool CodeGeneratorFile::CloseBlock() {
    if (block_positions_.empty()) return false;
    block_positions_.pop_back();
    Line("}");
    return true;
}

bool CodeGeneratorFile::CloseAllBlocks() {
    bool to_return = !block_positions_.empty();
    while (!block_positions_.empty()) CloseBlock();
    return to_return;
}

bool CodeGeneratorFile::CloseAndOpenBlock(const std::string &block_header) {
    if (block_positions_.empty()) return false;
    block_positions_.pop_back();
    Line("}");
    return OpenBlock(block_header);
}

void CodeGeneratorFile::SetIndentLevel(int level) {
    if (level <= 0) {
        throw code_generation::IndentationError(
            "CodeGeneratorFile::SetIndentLevel", level);
    }
    indent_level_ = level;
}

size_t CodeGeneratorFile::SetCursor(int position) {
    cursor_ = GetContainedPosition(position);
    return cursor_;
}

int CodeGeneratorFile::MoveCursor(int offset) {
    int next_pos = cursor_ + offset;
    return SetCursor(next_pos);
}

std::string CodeGeneratorFile::GetContent() const {
    std::string content = "";
    for (const std::string &line : content_) content += line;
    return content;
}

size_t CodeGeneratorFile::GetContainedPosition(int position) const {
    if (content_.empty()) return 0;
    while (position < 0) position += content_.size() + 1;
    size_t pos_not_negative = static_cast<size_t>(position);
    while (pos_not_negative > content_.size()) {
        pos_not_negative -= content_.size() + 1;
    }
    return pos_not_negative;
}

size_t CodeGeneratorFile::GetPositionStartBlock() const {
    if (block_positions_.empty()) {
        throw code_generation::CodeGenerationError(
            "CodeGeneratorFile::GetPositionStartBlock: No open blocks");
    }
    size_t last_block_pos = block_positions_.back();
    return last_block_pos + 1;
}

}  // namespace code_generation