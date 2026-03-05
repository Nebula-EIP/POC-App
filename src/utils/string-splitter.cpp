#include "utils/string-splitter.hpp"

namespace utils {
std::vector<std::string> SplitByDelims(
    const std::string& input,
    const std::vector<char>& delims,
    bool keep_empty,
    bool keep_delims,
    bool ignore_delims_in_quotes
) {
    // If no delimiters are provided, return the whole string as a single token.
    if (delims.empty())
        return { input };

    // O(1) average lookup to test if a character is a delimiter.
    std::unordered_set<char> delim_set(delims.begin(), delims.end());

    std::vector<std::string> out;
    std::string current;
    current.reserve(input.size());

    bool in_single_quotes = false;
    bool in_double_quotes = false;
    bool escaping = false;

    for (char c : input) {
        if (escaping) {
            current.push_back(c);
            escaping = false;
            continue;
        }

        if ((in_single_quotes || in_double_quotes) && c == '\\') {
            current.push_back(c);
            escaping = true;
            continue;
        }

        if (c == '\'' && !in_double_quotes) {
            in_single_quotes = !in_single_quotes;
            current.push_back(c);
            continue;
        }

        if (c == '"' && !in_single_quotes) {
            in_double_quotes = !in_double_quotes;
            current.push_back(c);
            continue;
        }

        const bool canSplit = !ignore_delims_in_quotes || (!in_single_quotes && !in_double_quotes);

        if (canSplit && delim_set.find(c) != delim_set.end()) {
            // We hit a delimiter: flush the current token.
            if (keep_delims)
                current.push_back(c);
            if (keep_empty || !current.empty()) {
                out.push_back(current);
            }
            current.clear();
        } else {
            // Regular character: accumulate into the current token.
            current.push_back(c);
        }
    }

    // Flush the last token.
    if (keep_empty || !current.empty())
        out.push_back(current);

    return out;
}
}  // namespace utils