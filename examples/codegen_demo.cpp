#include <filesystem>
#include <iostream>

#include "demo_support.hpp"
#include "mega_flow_graph.hpp"
#include "src/graph-exporter/graph_exporter.hpp"

int main() {
    core::Graph graph;
    BuildMegaFlowGraph(graph);

    graph_exporter::GraphExporter exporter;
    const auto repo_root = FindRepoRoot();
    const auto output_dir = repo_root / "examples" / "output";

    const auto export_result = exporter.ExportCpp(
        graph, output_dir, "mega_flow_graph", false, false, false);
    if (!export_result.has_value()) {
        std::cerr << "[ERROR] " << export_result.error() << '\n';
        return 1;
    }

    const auto cpp_path = *export_result;
    const auto bin_path = output_dir / "mega_flow_graph_demo";

    int compile_exit = 0;
    const auto compile_output = RunCommandCaptureOutput(
        "g++ -std=c++23 \"" + cpp_path.string() + "\" -o \"" +
            bin_path.string() + "\" 2>&1",
        compile_exit);
    if (compile_exit != 0) {
        std::cerr << "[ERROR] Compile failed\n" << compile_output << '\n';
        return 2;
    }

    int run_exit = 0;
    const auto run_output = RunCommandCaptureOutput(
        "\"" + bin_path.string() + "\" 2>&1", run_exit);
    if (run_exit != 0) {
        std::cerr << "[ERROR] Run failed\n" << run_output << '\n';
        return 3;
    }

    std::cout << run_output;
    return 0;
}