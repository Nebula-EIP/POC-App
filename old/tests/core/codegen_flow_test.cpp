#include <array>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include "mega_flow_graph.hpp"
#include "editor/code-generation/codegen-context.hpp"
#include "graph-exporter/graph_exporter.hpp"

namespace {

inline std::string RunCommandCaptureOutput(const std::string &command,
                                           int &exit_code) {
    std::array<char, 256> buffer{};
    std::string output;

    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        exit_code = -1;
        return "";
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) {
        output += buffer.data();
    }

    exit_code = pclose(pipe);
    return output;
}

inline std::filesystem::path FindRepoRoot() {
    try {
        std::filesystem::path exe = std::filesystem::read_symlink("/proc/self/exe");
        if (exe.is_relative()) exe = std::filesystem::absolute(exe);
        std::filesystem::path p = exe.parent_path();
        while (!p.empty() && p != p.root_path()) {
            if (std::filesystem::exists(p / "CMakeLists.txt")) return p;
            p = p.parent_path();
        }
    } catch (...) {
    }

    std::filesystem::path cur = std::filesystem::current_path();
    while (!cur.empty() && cur != cur.root_path()) {
        if (std::filesystem::exists(cur / "CMakeLists.txt")) return cur;
        cur = cur.parent_path();
    }

    return std::filesystem::current_path();
}

}  // namespace

TEST(CodegenDemoTest, GeneratesAndRunsSingleLinearGraph) {
    core::Graph graph;
    BuildMegaFlowGraph(graph);

    editor::code_generation::CodegenContext context;
    const auto content = context.Generate(graph, false).GetFormatedContent();

    EXPECT_NE(content.find("if ("), std::string::npos);
    EXPECT_NE(content.find("for (int i = 1"), std::string::npos);
    EXPECT_NE(content.find("while ("), std::string::npos);
    EXPECT_NE(content.find("std::cout"), std::string::npos);
    EXPECT_NE(content.find("return 0;"), std::string::npos);

    const auto repo_root = FindRepoRoot();
    const auto output_dir = repo_root / "examples" / "output";

    graph_exporter::GraphExporter exporter;
    const auto export_result = exporter.ExportCpp(graph, output_dir,
                                                  "mega_flow_graph_test",
                                                  false, false, false);
    ASSERT_TRUE(export_result.has_value()) << export_result.error();

    const auto cpp_path = *export_result;
    const auto bin_path = output_dir / "mega_flow_graph_test";

    int compile_exit = 0;
    const auto compile_output = RunCommandCaptureOutput(
        "g++ -std=c++23 \"" + cpp_path.string() + "\" -o \"" +
            bin_path.string() + "\" 2>&1",
        compile_exit);
    ASSERT_EQ(compile_exit, 0) << compile_output;

    int run_exit = 0;
    const auto run_output = RunCommandCaptureOutput(
        "\"" + bin_path.string() + "\" 2>&1", run_exit);
    ASSERT_EQ(run_exit, 0) << run_output;

    EXPECT_NE(run_output.find("validation ok"), std::string::npos);
    EXPECT_EQ(run_output.find("validation error"), std::string::npos);
    EXPECT_EQ(run_output.find("while body"), std::string::npos);
    EXPECT_NE(run_output.find("4"), std::string::npos);
    EXPECT_NE(run_output.find("8"), std::string::npos);
    EXPECT_NE(run_output.find("16"), std::string::npos);
    EXPECT_NE(run_output.rfind("12"), std::string::npos);
}