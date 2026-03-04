#include "code-generation/code-generator-file.hpp"

namespace code_generation {
int CodeGeneratorFile::AddContent(const std::string& line)
{
    if (cursor_ > content_.size())
        cursor_ = content_.size();
    content_.insert(cursor_, line);
    cursor_ += line.length();
    return cursor_;
}

int CodeGeneratorFile::AddContentAt(const std::string& line, int position)
{
    if (position < 0 || position > content_.length())
        throw code_generation::CursorOutOfBoundsError(
            "CodeGeneratorFile::AddContentAt", position,
            static_cast<long long>(content_.length()));
    content_.insert(position, line);
    cursor_ = position + line.length();
    return cursor_;
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
    if (position < 0 || position > content_.length())
        throw code_generation::CursorOutOfBoundsError(
            "CodeGeneratorFile::SetCursor", position,
            static_cast<long long>(content_.length()));
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
    return content_;
}
}  // namespace code_generation