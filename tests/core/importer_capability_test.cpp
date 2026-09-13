/**
 * @file importer_capability_test.cpp
 * @brief Tests for the IImporterCapability interface.
 *
 * @author Created by Nolan Papa
 * @date Created on 06-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 06-09-2026
 */

#include <gtest/gtest.h>

#include <type_traits>

#include "exception/graph_exception/connection_exception.hpp"
#include "exception/graph_exception/node_exception.hpp"
#include "graph/graph.hpp"
#include "modules/capabilities/importer_capability.hpp"

namespace {

TEST(ImporterCapabilityTest, IsAnOptionalCoreCapability) {
    static_assert(std::is_base_of_v<core::ICapability,
                                    core::capa::IImporterCapability>);
    static_assert(std::is_abstract_v<core::capa::IImporterCapability>);
    static_assert(std::is_aggregate_v<core::capa::ImportRequest>);

    SUCCEED();
}

TEST(ImporterCapabilityTest, ReportsImportErrorsWithoutThrowing) {
    class FailingImporter final : public core::capa::IImporterCapability {
       public:
        std::expected<void, core::capa::ImportError> importCode(
            const core::capa::ImportRequest &request) const override {
            EXPECT_EQ(request.source, "bad;");
            return std::unexpected(core::capa::ImportError{
                core::capa::ImportEntityKind::kSource, 0, 4,
                "Unsupported source syntax"});
        }
    } importer;

    core::Graph graph;
    const auto result = importer.importCode({graph, "bad;"});

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().kind, core::capa::ImportEntityKind::kSource);
    EXPECT_EQ(result.error().source_offset, 4U);
    EXPECT_EQ(result.error().message, "Unsupported source syntax");
    EXPECT_TRUE(graph.GetAllNodes().empty());
}

TEST(ImporterCapabilityTest, ImportsNodesPinsAndConnectionsIntoTheGraph) {
    class GraphImporter final : public core::capa::IImporterCapability {
       public:
        std::expected<void, core::capa::ImportError> importCode(
            const core::capa::ImportRequest &request) const override {
            auto &source = request.graph.CreateNode(1);
            auto &sink = request.graph.CreateNode(2);
            const auto output =
                request.graph.AddOutputPin(source.Id(), "value", 7);
            const auto input = request.graph.AddInputPin(sink.Id(), "value", 7);
            request.graph.Connect(source.Id(), output, sink.Id(), input);
            return {};
        }
    } importer;

    core::Graph graph;
    const auto result = importer.importCode({graph, "source -> sink"});

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(graph.GetAllNodes().size(), 2U);
    ASSERT_EQ(graph.GetAllConnections().size(), 1U);
    EXPECT_EQ(graph.GetAllConnections().begin()->second.data_type_, 7U);
}

TEST(ImporterCapabilityTest, PreservesGraphExceptionsForInvalidImports) {
    class InvalidImporter final : public core::capa::IImporterCapability {
       public:
        std::expected<void, core::capa::ImportError> importCode(
            const core::capa::ImportRequest &request) const override {
            (void)request.graph.Connect(1, 1, 2, 1);
            return {};
        }
    } importer;

    core::Graph graph;

    EXPECT_THROW(
        [&] { (void)importer.importCode({graph, "invalid connection"}); }(),
        core::NodeNotFoundException);
}

TEST(ImporterCapabilityTest, DelegatesParsingToTheConfiguredHandler) {
    bool handler_called = false;
    core::capa::ImporterCapability importer(
        [&](const core::capa::ImportRequest &request)
            -> std::expected<void, core::capa::ImportError> {
            handler_called = true;
            EXPECT_EQ(request.source, "node");
            request.graph.CreateNode(3);
            return {};
        });

    core::Graph graph;
    const auto result = importer.importCode({graph, "node"});

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(handler_called);
    ASSERT_EQ(graph.GetAllNodes().size(), 1U);
    EXPECT_EQ(graph.GetAllNodes().begin()->second.Type(), 3U);
}

TEST(ImporterCapabilityTest, ReportsAnErrorWhenNoHandlerIsConfigured) {
    core::capa::ImporterCapability importer;
    core::Graph graph;

    EXPECT_FALSE(importer.hasImportHandler());
    const auto result = importer.importCode({graph, "node"});

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().kind, core::capa::ImportEntityKind::kSource);
    EXPECT_EQ(result.error().source_offset, 0U);
    EXPECT_EQ(result.error().message, "No importer handler is configured");

    importer.setImportHandler(
        [](const core::capa::ImportRequest &)
            -> std::expected<void, core::capa::ImportError> { return {}; });

    EXPECT_TRUE(importer.hasImportHandler());
    EXPECT_TRUE(importer.importCode({graph, "node"}).has_value());
}

}  // namespace