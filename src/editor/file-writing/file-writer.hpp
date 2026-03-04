#pragma once
#include <string>

namespace file_writing {
class FileWriter {
 public:
    FileWriter() = default;

    /**
     * @brief Writes the given content to a file at the specified path.
     * @param path The path to the file where the content should be written.
     * @param content The content to be written to the file.
     * @return True if the file was successfully written, false otherwise (e.g., if there was an error during writing).
     */
    bool WriteToFile(const std::string& path, const std::string& content);
};
}  // namespace file_writing
