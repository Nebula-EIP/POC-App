#include <gtest/gtest.h>

#include "node_catalog/node_catalog.hpp"
#include "modules/loader.hpp"
#include "exception/module_exception/module_validation_exception.hpp"

using namespace core;

class NodeCatalogTest : public ::testing::Test {
protected:
    ModuleLoader loader_;
    NodeCatalog catalog_;
};

TEST_F(NodeCatalogTest, InitialCatalogIsEmpty) {
    catalog_.Update(loader_);
    EXPECT_TRUE(catalog_.GetNodes().empty());
}

TEST_F(NodeCatalogTest, PopulatesFromValidModule) {
    ModuleId id = loader_.Load(NEBULA_FIXTURE_VALID);
    catalog_.Update(loader_);
    
    auto nodes = catalog_.GetNodes();
    EXPECT_FALSE(nodes.empty());
    
    // Valid module provides exactly one node type based on fixture implementations
    auto node_by_mod = catalog_.GetNodesByModule(id);
    EXPECT_EQ(node_by_mod.size(), nodes.size());

    if (!nodes.empty()) {
        auto node = nodes.front();
        auto found = catalog_.GetNode(node.id_);
        EXPECT_TRUE(found.has_value());
        EXPECT_EQ(found->id_, node.id_);
        EXPECT_EQ(found->module_id_, id);
    }
}

TEST_F(NodeCatalogTest, ClearsOnModuleUnload) {
    ModuleId id = loader_.Load(NEBULA_FIXTURE_VALID);
    catalog_.Update(loader_);
    EXPECT_FALSE(catalog_.GetNodes().empty());

    loader_.Unload(id);
    catalog_.Update(loader_);
    EXPECT_TRUE(catalog_.GetNodes().empty());
}

TEST_F(NodeCatalogTest, EmptyModuleYieldsEmptyCatalog) {
    // The loader rejects modules without the mandatory node capability.
    // However, if we try to load it and it fails, the catalog should still be empty.
    EXPECT_THROW(loader_.Load(NEBULA_FIXTURE_NO_NODES), InvalidModuleException);
    catalog_.Update(loader_);
    EXPECT_TRUE(catalog_.GetNodes().empty());
}
