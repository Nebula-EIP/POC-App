#include "code-generation/code-generator-file.hpp"

namespace code_generation {
int CodeGeneratorFile::Line(const std::string& line)
{
    LineAt(line, cursor_);
    return cursor_;
}

int CodeGeneratorFile::LineAt(const std::string& line, int position)
{
    position = GetContainedPosition(position);
    content_.insert(content_.begin() + position, line);
    if (position <= cursor_)
        cursor_++;

    return cursor_;
}

bool CodeGeneratorFile::OpenBlock(const std::string& block_header)
{
    block_positions_.push_back(cursor_);
    Line(block_header + " {");
    return true;
}

bool CodeGeneratorFile::CloseBlock()
{
    if (block_positions_.empty())
        return false;
    block_positions_.pop_back();
    Line("}");
    return true;
}

void CodeGeneratorFile::SetIndentLevel(int level)
{
    if (level <= 0)
        throw code_generation::IndentationError(
            "CodeGeneratorFile::SetIndentLevel", level);
    indent_level_ = level;
}

int CodeGeneratorFile::SetCursor(int position)
{
    cursor_ = position;
    return cursor_;
}

int CodeGeneratorFile::MoveCursor(int offset)
{
    int nextPos = cursor_ + offset;
    return SetCursor(nextPos);
}

std::string CodeGeneratorFile::GetContent() const
{
    std::string content = "";
    for (const std::string& line : content_)
        content += line;
    return content;
}

int CodeGeneratorFile::GetContainedPosition(int position) const
{
    if (content_.empty())
        return 0;
    while (position < 0)
        position += content_.size() + 1;
    while (position > content_.size())
        position -= content_.size() + 1;
    return position;
}

int CodeGeneratorFile::GetPositionStartBlock() const
{
    if (block_positions_.empty())
        return -1;
    return block_positions_.back() + 1;
}

}  // namespace code_generation