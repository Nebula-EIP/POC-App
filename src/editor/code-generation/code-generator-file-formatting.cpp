#include "code-generation/code-generator-file.hpp"
#include "utils/string-splitter.hpp"
#include <algorithm>

namespace code_generation {
namespace {
int CountCharOutsideQuotes(const std::string& s, char needle)
{
    bool in_single_quotes = false;
    bool in_double_quotes = false;
    bool escaping = false;
    int count = 0;

    for (char c : s) {
        if (escaping) {
            escaping = false;
            continue;
        }

        if ((in_single_quotes || in_double_quotes) && c == '\\') {
            escaping = true;
            continue;
        }

        if (c == '\'' && !in_double_quotes) {
            in_single_quotes = !in_single_quotes;
            continue;
        }
        if (c == '"' && !in_single_quotes) {
            in_double_quotes = !in_double_quotes;
            continue;
        }

        if (!in_single_quotes && !in_double_quotes && c == needle)
            ++count;
    }
    return count;
}

std::vector<std::string> SplitByNewlines(const std::string& s)
{
    std::vector<std::string> sub_lines = utils::SplitByDelims(s, {';'}, false, true);
    for (size_t i = 0; i < sub_lines.size(); i++) {
        if (std::find(sub_lines[i].begin(), sub_lines[i].end(), '\n') == sub_lines[i].end())
            continue;

        std::vector<std::string> sub_sub_lines = utils::SplitByDelims(sub_lines[i], {'\n'}, false, true);
        sub_lines[i] = "";
        for (std::string &sub_sub_line : sub_sub_lines) {
            if (!sub_sub_line.empty())
                sub_lines[i] += sub_sub_line;
        }
    }
    return sub_lines;
}

std::string GetNewLIneFormatted(const std::string& line, int indent_level, int current_indent)
{
    std::string final_line = "";
    for (int i = 0; i < current_indent; i++)
        final_line += std::string(indent_level, ' ');
    final_line += line + "\n";
    return final_line;
}

}  // namespace

std::string CodeGeneratorFile::GetFormatedContent() const
{
    std::string final_code = "";
    std::string content = GetContent();
    // Split the content into code blocks based on curly braces, keeping the delimiters to determine indentation levels.
    std::vector<std::string> code_blocks = utils::SplitByDelims(content, {'{', '}'}, false, true);
    int current_indent = 0;

    for (std::string &line : code_blocks) {
        std::vector<std::string> sub_lines = SplitByNewlines(line);

        // Parse each line in the code block, applying the current indentation
        for (std::string &sub_line : sub_lines) {
            current_indent -= CountCharOutsideQuotes(sub_line, '}');
            final_code += GetNewLIneFormatted(sub_line, indent_level_, current_indent);
            current_indent += CountCharOutsideQuotes(sub_line, '{');
        }
    }
    return final_code;
}
}  // namespace code_generation