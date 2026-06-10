#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "core/graph.hpp"
#include "core/nodes/literal_node.hpp"
#include "core/nodes/print_node.hpp"
#include "graph-exporter/graph_exporter.hpp"

namespace {
namespace fs = std::filesystem;

std::string ReadAll(const fs::path &path) {
    std::ifstream in(path);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

class GraphExporterTest : public ::testing::Test {
   protected:
    fs::path test_dir_;

    void SetUp() override {
        test_dir_ = fs::temp_directory_path() / "nebula_graph_exporter_tests" /
                    ::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name() /
                    ::testing::UnitTest::GetInstance()->current_test_info()->name();
        fs::remove_all(test_dir_);
        fs::create_directories(test_dir_);
    }

    void TearDown() override { fs::remove_all(test_dir_); }
};

TEST_F(GraphExporterTest, ExportCppWritesFileInNestedDirectory) {
    core::Graph graph;
    auto *message = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *printer = graph.AddNode<core::PrintNode>(core::NodeBase::NodeKind::kPrint, {0, 0});

    message->SetType(core::NodeBase::PinDataType::kString);
    message->set_data(std::string("Exported from GraphExporter"));
    graph.Link(message, 0, printer, 1);

    graph_exporter::GraphExporter exporter;
    const fs::path output_dir = test_dir_ / "nested" / "output";
    const auto result = exporter.ExportCpp(graph, output_dir, "demo_export", false, false);

    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(fs::exists(*result));
    EXPECT_EQ(result->extension(), ".cpp");
    EXPECT_EQ(result->parent_path(), output_dir);

    const std::string content = ReadAll(*result);
    EXPECT_NE(content.find("Exported from GraphExporter"), std::string::npos);
    EXPECT_NE(content.find("std::cout"), std::string::npos);
}

TEST_F(GraphExporterTest, ExportCppRejectsEmptyFileStem) {
    core::Graph graph;
    graph_exporter::GraphExporter exporter;

    const auto result = exporter.ExportCpp(graph, test_dir_, "", false, false);

    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("File stem cannot be empty"), std::string::npos);
}

}  // namespace
