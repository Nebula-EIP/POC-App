#include "code-generation/code-generator-file.hpp"

namespace code_generation {
int CodeGeneratorFile::Line(const std::string& line)
{
    LineAt(line, cursor_);
    cursor_++;
    return cursor_;
}

int CodeGeneratorFile::LineAt(const std::string& line, int position)
{
    position = GetContainedPosition(position);
    content_.insert(content_.begin() + position, line);
    return cursor_;
}

bool CodeGeneratorFile::OpenBlock(const std::string& block_header)
{
    open_blocks_++;
    Line(block_header + " {");
    return true;
}

bool CodeGeneratorFile::CloseBlock()
{
    if (open_blocks_ <= 0)
        return false;
    open_blocks_--;
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

}  // namespace code_generation