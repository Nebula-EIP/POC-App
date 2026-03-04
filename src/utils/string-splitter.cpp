#include "utils/string-splitter.hpp"

namespace utils {
std::vector<std::string> splitByDelims(
    const std::string& input,
    const std::vector<char>& delims,
    bool keepEmpty,
    bool keepDelims,
    bool ignoreDelimsInQuotes
) {
    // If no delimiters are provided, return the whole string as a single token.
    if (delims.empty())
        return { input };

    // O(1) average lookup to test if a character is a delimiter.
    std::unordered_set<char> delimSet(delims.begin(), delims.end());

    std::vector<std::string> out;
    std::string current;
    current.reserve(input.size());

    bool inSingleQuotes = false;
    bool inDoubleQuotes = false;
    bool escaping = false;

    for (char c : input) {
        if (escaping) {
            current.push_back(c);
            escaping = false;
            continue;
        }

        if ((inSingleQuotes || inDoubleQuotes) && c == '\\') {
            current.push_back(c);
            escaping = true;
            continue;
        }

        if (c == '\'' && !inDoubleQuotes) {
            inSingleQuotes = !inSingleQuotes;
            current.push_back(c);
            continue;
        }

        if (c == '"' && !inSingleQuotes) {
            inDoubleQuotes = !inDoubleQuotes;
            current.push_back(c);
            continue;
        }

        const bool canSplit = !ignoreDelimsInQuotes || (!inSingleQuotes && !inDoubleQuotes);

        if (canSplit && delimSet.find(c) != delimSet.end()) {
            // We hit a delimiter: flush the current token.
            if (keepDelims)
                current.push_back(c);
            if (keepEmpty || !current.empty()) {
                out.push_back(current);
            }
            current.clear();
        } else {
            // Regular character: accumulate into the current token.
            current.push_back(c);
        }
    }

    // Flush the last token.
    if (keepEmpty || !current.empty())
        out.push_back(current);

    return out;
}
}  // namespace utils