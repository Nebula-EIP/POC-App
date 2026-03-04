#include "code-generation/code-generator-file.hpp"
#include "utils/string-splitter.hpp"

namespace code_generation {
namespace {
int countCharOutsideQuotes(const std::string& s, char needle)
{
    bool inSingleQuotes = false;
    bool inDoubleQuotes = false;
    bool escaping = false;
    int count = 0;

    for (char c : s) {
        if (escaping) {
            escaping = false;
            continue;
        }

        if ((inSingleQuotes || inDoubleQuotes) && c == '\\') {
            escaping = true;
            continue;
        }

        if (c == '\'' && !inDoubleQuotes) {
            inSingleQuotes = !inSingleQuotes;
            continue;
        }
        if (c == '"' && !inSingleQuotes) {
            inDoubleQuotes = !inDoubleQuotes;
            continue;
        }

        if (!inSingleQuotes && !inDoubleQuotes && c == needle)
            ++count;
    }
    return count;
}

std::vector<std::string> splitByNewlines(const std::string& s)
{
    std::vector<std::string> sub_lines = utils::splitByDelims(s, {';'}, true, true);
    for (int i = 0; i < sub_lines.size(); i++) {
        if (std::find(sub_lines[i].begin(), sub_lines[i].end(), '\n') == sub_lines[i].end())
            continue;

        std::vector<std::string> sub_sub_lines = utils::splitByDelims(sub_lines[i], {'\n'}, true, true);
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
    // Split the content into code blocks based on curly braces, keeping the delimiters to determine indentation levels.
    std::vector<std::string> code_blocks = utils::splitByDelims(content_, {'{', '}'}, true, true);
    int current_indent = 0;

    for (std::string &line : code_blocks) {
        std::vector<std::string> sub_lines = splitByNewlines(line);

        // Parse each line in the code block, applying the current indentation
        for (std::string &sub_line : sub_lines) {
            current_indent -= countCharOutsideQuotes(sub_line, '}');
            final_code += GetNewLIneFormatted(sub_line, indent_level_, current_indent);
            current_indent += countCharOutsideQuotes(sub_line, '{');
        }
    }
    return final_code;
}
}  // namespace code_generation