#pragma once
#include <string>
#include <vector>
#include <unordered_set>

namespace utils {
std::vector<std::string> splitByDelims(
    const std::string& input,
    const std::vector<char>& delims,
    bool keepEmpty = false,
    bool keepDelims = false,
    bool ignoreDelimsInQuotes = true
);
}  // namespace utils