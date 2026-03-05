#pragma once
#include <string>
#include <vector>
#include <unordered_set>

namespace utils {

/**
 * @brief Splits the input string into a vector of substrings based on the specified delimiters.
 * @param input The string to be split.
 * @param delims A vector of characters to be used as delimiters for splitting the input string.
 * @param keep_empty If true, empty substrings resulting from consecutive delimiters will be included
 * in the output vector. If false, they will be excluded.
 * @param keep_delims If true, the delimiters themselves will be included as separate substrings in the output vector.
 * If false, the delimiters will not be included in the output vector.
 * @param ignore_delims_in_quotes If true, delimiters that appear within quoted substrings
 * will be ignored and not treated as split points. If false, delimiters will be treated as split points regardless of their position in the input string.
 * @return A vector of substrings resulting from splitting the input string based on the specified delimit
 * ers and options.
 */
std::vector<std::string> SplitByDelims(
    const std::string& input,
    const std::vector<char>& delims,
    bool keep_empty = false,
    bool keep_delims = false,
    bool ignore_delims_in_quotes = true
);
}  // namespace utils