#pragma once

#include <filesystem>
#include <expected>
#include <string>
#include <vector>

#include "editor/code-generation/codegen-context.hpp"
#include "editor/file-writing/file-writer.hpp"

namespace graph_exporter {

class GraphExporter {
   public:
    GraphExporter() = default;

    std::expected<std::filesystem::path, std::string> ExportCpp(
        const core::Graph &graph, const std::filesystem::path &output_directory,
        const std::string &file_stem, bool with_outputs = true,
        bool print_all_results = false, bool fold_constants = false) const;

   private:
    static std::expected<std::filesystem::path, std::string> ValidateRequest(
        const std::filesystem::path &output_directory,
        const std::string &file_stem);

    static std::string BuildSourceText(
        const ::code_generation::CodeGeneratorFile &generated_file);
};

}  // namespace graph_exporter
