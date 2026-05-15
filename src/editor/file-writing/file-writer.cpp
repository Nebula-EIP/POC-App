#include "file-writing/file-writer.hpp"

#include <filesystem>
#include <fstream>

namespace file_writing {
bool FileWriter::WriteToFile(const std::string &path,
                             const std::string &content) {
    // Create parent directories if they don't exist
    std::filesystem::path file_path(path);
    if (file_path.has_parent_path()) {
        std::filesystem::create_directories(file_path.parent_path());
    }

    // Open the file for writing (creates it if it doesn't exist, truncates it
    // if it does)
    std::ofstream output_file(path);
    if (!output_file.is_open()) return false;
    // Write the content to the file
    output_file << content;
    // Check if the write operation was successful
    if (output_file.fail()) return false;
    output_file.close();
    return true;
}
}  // namespace file_writing
