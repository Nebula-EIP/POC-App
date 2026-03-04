#include "code-generation/code-generator-file.hpp"
#include "utils/string-splitter.hpp"

namespace code_generation {
std::string CodeGeneratorFile::GetFormatedContent() const
{
    std::string final_code = "";
    // Split the content into code blocks based on curly braces, keeping the delimiters to determine indentation levels.
    std::vector<std::string> code_blocks = utils::splitByDelims(content_, {'{', '}'}, true, true);

    // Track the current indentation level based on the presence of opening and closing braces.
    int current_indent = 0;

    for (std::string &line : code_blocks) {
        std::vector<std::string> sub_lines = utils::splitByDelims(line, {';'}, true, true);

        // if the line contains a closing brace, decrease the indentation level
        if (line.find('}') != std::string::npos)
            current_indent--;
        // Parse each line in the code block, applying the current indentation
        for (std::string &sub_line : sub_lines) {
            for (int i = 0; i < current_indent; i++)
                final_code += std::string(indent_level_, ' ');
            final_code += sub_line + "\n";
        }
        // if the line contains an opening brace, increase the indentation
        if (line.find('{') != std::string::npos)
            current_indent++;
    }
    return final_code;
}
}  // namespace code_generation