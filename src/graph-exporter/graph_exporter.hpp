#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

#include "editor/code-generation/codegen-context.hpp"
#include "editor/file-writing/file-writer.hpp"

namespace graph_exporter {

class GraphExporter {
   public:
    GraphExporter() = default;

    /**
     * @brief Exports the given graph to a C++ source file in the specified
     * output directory with the given file stem.
     * @param graph The graph to export
     * @param output_directory The directory where the generated C++ file will
     * be saved
     * @param file_stem The stem of the generated C++ file (without extension)
     * @param with_outputs If true, includes code to print the outputs of the
     * graph
     * @param print_all_results If true, adds print statements for all node
     * outputs
     * @param fold_constants If true, performs constant folding optimization
     * during code generation
     * @return An expected containing the path to the generated C++ file on
     * success, or
     */
    std::expected<std::filesystem::path, std::string> ExportCpp(
        const core::Graph &graph, const std::filesystem::path &output_directory,
        const std::string &file_stem, bool with_outputs = true,
        bool print_all_results = false, bool fold_constants = false) const;

   private:
    /**
     * @brief Validates the export request parameters.
     * @param output_directory The directory where the generated C++ file will
     * be saved
     * @param file_stem The stem of the generated C++ file (without extension)
     * @return An expected containing the path to the generated C++ file on
     * success, or an error message on failure
     */
    static std::expected<std::filesystem::path, std::string> ValidateRequest(
        const std::filesystem::path &output_directory,
        const std::string &file_stem);

    /**
     * @brief Builds the source text for the generated C++ file from the given
     * CodeGeneratorFile.
     * @param generated_file The CodeGeneratorFile containing the generated code
     * structure
     * @return A string containing the complete source code to be written to the
     * C++ file
     */
    static std::string BuildSourceText(
        const ::code_generation::CodeGeneratorFile &generated_file);
};

}  // namespace graph_exporter
