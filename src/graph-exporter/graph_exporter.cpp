#include "graph-exporter/graph_exporter.hpp"

#include <filesystem>
#include <sstream>

namespace graph_exporter {

std::string GraphExporter::BuildSourceText(
    const ::code_generation::CodeGeneratorFile &generated_file) {
    std::ostringstream oss;
    const auto &lines = generated_file.GetLines();
    for (const auto &line : lines) {
        oss << line << '\n';
    }
    return oss.str();
}

std::expected<std::filesystem::path, std::string> GraphExporter::ValidateRequest(
    const std::filesystem::path &output_directory,
    const std::string &file_stem) {
    if (file_stem.empty()) {
        return std::unexpected(std::string{"File stem cannot be empty"});
    }

    if (output_directory.empty()) {
        return std::unexpected(std::string{"Output directory cannot be empty"});
    }

    return output_directory / (file_stem + ".cpp");
}

std::expected<std::filesystem::path, std::string> GraphExporter::ExportCpp(
    const core::Graph &graph, const std::filesystem::path &output_directory,
    const std::string &file_stem, bool with_outputs,
    bool print_all_results, bool fold_constants) const {
    const auto kTargetPath = ValidateRequest(output_directory, file_stem);
    if (!kTargetPath.has_value()) {
        return std::unexpected(kTargetPath.error());
    }

    editor::code_generation::CodegenContext codegen_context;
    const auto kGeneratedFile = with_outputs
                                    ? codegen_context.GenerateWithOutputs(
                                graph, print_all_results,
                                fold_constants)
                            : codegen_context.Generate(graph,
                                              fold_constants);

    file_writing::FileWriter writer;
    if (!writer.WriteToFile(kTargetPath->string(),
                            BuildSourceText(kGeneratedFile))) {
        return std::unexpected(std::string{"Failed to write generated file: "} +
                               kTargetPath->string());
    }

    return *kTargetPath;
}

}  // namespace graph_exporter
