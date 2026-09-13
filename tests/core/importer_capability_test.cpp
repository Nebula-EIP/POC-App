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

#include "core/graph/graph.hpp"
#include "core/modules/capabilities/importer_capability.hpp"

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
            const core::capa::ImportRequest &) const override {
            return std::unexpected(core::capa::ImportError{
                core::capa::ImportEntityKind::kSource, 0, 4,
                "Unsupported source syntax"});
        }
    } importer;

    const core::capa::ImportError error{
        core::capa::ImportEntityKind::kSource, 0, 4,
        "Unsupported source syntax"};

    EXPECT_EQ(error.kind, core::capa::ImportEntityKind::kSource);
    EXPECT_EQ(error.source_offset, 4U);
    EXPECT_EQ(error.message, "Unsupported source syntax");
}

}  // namespace