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
}  // namespace

std::string CodeGeneratorFile::GetFormatedContent() const
{
    std::string final_code = "";
    // Split the content into code blocks based on curly braces, keeping the delimiters to determine indentation levels.
    std::vector<std::string> code_blocks = utils::splitByDelims(content_, {'{', '}'}, true, true);

    // Track the current indentation level based on the presence of opening and closing braces.
    int current_indent = 0;

    for (std::string &line : code_blocks) {
        std::vector<std::string> sub_lines = utils::splitByDelims(line, {';'}, true, true);

        // If the line contains a closing brace outside of quotes, decrease the indentation level.
        current_indent -= countCharOutsideQuotes(line, '}');
        // Parse each line in the code block, applying the current indentation
        for (std::string &sub_line : sub_lines) {
            for (int i = 0; i < current_indent; i++)
                final_code += std::string(indent_level_, ' ');
            final_code += sub_line + "\n";
        }
        // If the line contains an opening brace outside of quotes, increase the indentation.
        current_indent += countCharOutsideQuotes(line, '{');
    }
    return final_code;
}
}  // namespace code_generation