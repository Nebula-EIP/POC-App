/**
 * @file exporter_capability_test.cpp
 * @brief Tests for the IExporterCapability interface.
 *
 * @author Created by Nathan Bezard
 * @date Created on 07-09-2026
 *
 * @author Last modified by Nathan Bezard
 * @date Last modified on 07-09-2026
 */

#include <gtest/gtest.h>

#include <type_traits>

#include "src/new_core/graph/graph.hpp"
#include "src/new_core/modules/capabilities/exporter_capability.hpp"

using namespace core;
using namespace core::capa;

namespace {

TEST(ExporterCapabilityTest, IsAnOptionalCoreCapability) {
    static_assert(std::is_base_of_v<
                  core::ICapability,
                  core::capa::IExporterCapability>);

    static_assert(
        std::is_abstract_v<core::capa::IExporterCapability>);

    static_assert(
        std::is_aggregate_v<core::capa::TypeDescriptor>);

    static_assert(
        std::is_aggregate_v<core::capa::NodeDescriptor>);

    static_assert(
        std::is_aggregate_v<core::capa::ExportError>);

    static_assert(
        std::is_aggregate_v<core::capa::NodeExportRequest>);

    static_assert(
        std::is_aggregate_v<core::capa::ExportContext>);

    SUCCEED();
}

TEST(ExporterCapabilityTest, StoresTypeDescriptorInformation) {
    const core::capa::TypeDescriptor type{
        42,
        "TestType",
    };

    EXPECT_EQ(type.id, 42);
    EXPECT_EQ(type.name, "TestType");
}

TEST(ExporterCapabilityTest, StoresNodeDescriptorInformation) {
    const core::capa::NodeDescriptor node{
        42,
        "TestNode",
    };

    EXPECT_EQ(node.id, 42);
    EXPECT_EQ(node.name, "TestNode");
}

TEST(ExporterCapabilityTest, ReportsExportErrors) {
    const core::capa::ExportError error{
        core::capa::ExportEntityKind::kNode,
        42,
        "Unable to export node",
    };

    EXPECT_EQ(error.kind, core::capa::ExportEntityKind::kNode);
    EXPECT_EQ(error.entity_id, 42U);
    EXPECT_EQ(error.message, "Unable to export node");
}

TEST(ExporterCapabilityTest, ExportEntityKindContainsTypeAndNode) {
    const auto type = core::capa::ExportEntityKind::kType;
    const auto node = core::capa::ExportEntityKind::kNode;

    EXPECT_NE(type, node);
}

}  // namespace