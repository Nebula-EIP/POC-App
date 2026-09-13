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
        std::expected<void, core::capa::ImportError> ImportCode(
            const core::capa::ImportRequest &request) const override {
            EXPECT_EQ(request.source_, "bad;");
            return std::unexpected(core::capa::ImportError{
                core::capa::ImportEntityKind::kSource, 0, 4,
                "Unsupported source syntax"});
        }
    } importer_;

    core::Graph graph;
    const auto result = importer_.ImportCode({graph, "bad;"});

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().kind_, core::capa::ImportEntityKind::kSource);
    EXPECT_EQ(result.error().source_offset_, 4U);
    EXPECT_EQ(result.error().message_, "Unsupported source syntax");
    EXPECT_TRUE(graph.GetAllNodes().empty());
}

TEST(ImporterCapabilityTest, ImportsNodesPinsAndConnectionsIntoTheGraph) {
    class GraphImporter final : public core::capa::IImporterCapability {
       public:
        std::expected<void, core::capa::ImportError> ImportCode(
            const core::capa::ImportRequest &request) const override {
            auto &source = request.graph_.CreateNode(1);
            auto &sink = request.graph_.CreateNode(2);
            const auto output =
                request.graph_.AddOutputPin(source.Id(), "value", 7);
            const auto input = request.graph_.AddInputPin(sink.Id(), "value", 7);
            request.graph_.Connect(source.Id(), output, sink.Id(), input);
            return {};
        }
    } importer_;

    core::Graph graph;
    const auto result = importer_.ImportCode({graph, "source -> sink"});

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(graph.GetAllNodes().size(), 2U);
    ASSERT_EQ(graph.GetAllConnections().size(), 1U);
    EXPECT_EQ(graph.GetAllConnections().begin()->second.data_type_, 7U);
}

TEST(ImporterCapabilityTest, PreservesGraphExceptionsForInvalidImports) {
    class InvalidImporter final : public core::capa::IImporterCapability {
       public:
        std::expected<void, core::capa::ImportError> ImportCode(
            const core::capa::ImportRequest &request) const override {
            (void)request.graph_.Connect(1, 1, 2, 1);
            return {};
        }
    } importer_;

    core::Graph graph;

    EXPECT_THROW(
        [&] { (void)importer_.ImportCode({graph, "invalid connection"}); }(),
        core::NodeNotFoundException);
}

TEST(ImporterCapabilityTest, DelegatesParsingToTheConfiguredHandler) {
    bool handler_called = false;
    core::capa::ImporterCapability importer(
        [&](const core::capa::ImportRequest &request)
            -> std::expected<void, core::capa::ImportError> {
            handler_called = true;
            EXPECT_EQ(request.source_, "node");
            request.graph_.CreateNode(3);
            return {};
        });

    core::Graph graph;
    const auto result = importer.ImportCode({graph, "node"});

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(handler_called);
    ASSERT_EQ(graph.GetAllNodes().size(), 1U);
    EXPECT_EQ(graph.GetAllNodes().begin()->second.Type(), 3U);
}

TEST(ImporterCapabilityTest, ReportsAnErrorWhenNoHandlerIsConfigured) {
    core::capa::ImporterCapability importer;
    core::Graph graph;

    EXPECT_FALSE(importer.HasImportHandler());
    const auto result = importer.ImportCode({graph, "node"});

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().kind_, core::capa::ImportEntityKind::kSource);
    EXPECT_EQ(result.error().source_offset_, 0U);
    EXPECT_EQ(result.error().message_, "No importer handler is configured");

    importer.SetImportHandler(
        [](const core::capa::ImportRequest &)
            -> std::expected<void, core::capa::ImportError> { return {}; });

    EXPECT_TRUE(importer.HasImportHandler());
    EXPECT_TRUE(importer.ImportCode({graph, "node"}).has_value());
}

}  // namespace