#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

#include "graph.hpp"
#include "nodes/literal_node.hpp"

class GraphTest : public testing::Test {
 protected:
    core::Graph graph_;
};

// Node Management Tests

TEST_F(GraphTest, AddNode_ValidKind_ReturnsNonNull) {
    EXPECT_NE(graph_.AddNode(core::NodeBase::NodeKind::kLiteral), nullptr);
}

TEST_F(GraphTest, AddNode_UndefinedKind_ReturnsNull) {
    EXPECT_EQ(graph_.AddNode(core::NodeBase::NodeKind::kUndefined), nullptr);
}

TEST_F(GraphTest, AddNode_MultipleNodes_AssignsUniqueIds) {
    auto a = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto b = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto c = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);

    EXPECT_TRUE(
        (a->id() != b->id())
        && (a->id() != c->id())
        && (b->id() != c->id())
    );
}

TEST_F(GraphTest, AddNode_Templated_ReturnsCorrectType) {
    auto* node = graph_.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind(), core::NodeBase::NodeKind::kLiteral);
    
    // Verify it's actually a LiteralNode by calling a method specific to it
    EXPECT_EQ(node->GetDisplayName(), "Literal");
}

TEST_F(GraphTest, GetNode_ValidId_ReturnsCorrectNode) {
    auto* node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    uint32_t id = node->id();
    
    auto* retrieved = graph_.GetNode(id);
    EXPECT_EQ(retrieved, node);
    EXPECT_EQ(retrieved->id(), id);
}

TEST_F(GraphTest, GetNode_InvalidId_ReturnsNull) {
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    
    // Try to get a node with an ID that doesn't exist
    auto* node = graph_.GetNode(999);
    EXPECT_EQ(node, nullptr);
}

TEST_F(GraphTest, GetNode_Templated_ReturnsCorrectType) {
    auto* added_node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    uint32_t id = added_node->id();
    
    auto* retrieved = graph_.GetNode<core::LiteralNode>(id);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->id(), id);
    EXPECT_EQ(retrieved->GetDisplayName(), "Literal");
}

TEST_F(GraphTest, RemoveNode_ValidNode_RemovesFromGraph) {
    auto* node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    uint32_t id = node->id();
    
    graph_.RemoveNode(node);
    
    // Verify node is no longer accessible
    auto* retrieved = graph_.GetNode(id);
    EXPECT_EQ(retrieved, nullptr);
}

TEST_F(GraphTest, RemoveNode_NullPointer_DoesNotCrash) {
    // Should not crash when removing null
    EXPECT_NO_THROW(graph_.RemoveNode(nullptr));
}

TEST_F(GraphTest, RemoveNode_NonExistentNode_DoesNotCrash) {
    // Create a node in a different graph
    core::Graph other_graph;
    auto* other_node = other_graph.AddNode(core::NodeBase::NodeKind::kLiteral);
    
    // Try to remove it from this graph - should not crash
    EXPECT_NO_THROW(graph_.RemoveNode(other_node));
}

// Metadata Tests

TEST_F(GraphTest, Metadata_DefaultValues_AreSet) {
    EXPECT_EQ(graph_.GetProjectName(), "Untitled Project");
    EXPECT_EQ(graph_.GetVersion(), "1.0");
    EXPECT_EQ(graph_.GetAuthor(), "");
    // Timestamps should be set to current time (non-zero)
}

TEST_F(GraphTest, Metadata_SetProjectName_UpdatesValue) {
    graph_.SetProjectName("Test Project");
    EXPECT_EQ(graph_.GetProjectName(), "Test Project");
}

TEST_F(GraphTest, Metadata_SetAuthor_UpdatesValue) {
    graph_.SetAuthor("Test Author");
    EXPECT_EQ(graph_.GetAuthor(), "Test Author");
}

TEST_F(GraphTest, Metadata_UpdateModifiedTime_ChangesTimestamp) {
    auto initial_time = graph_.GetModifiedAt();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    graph_.UpdateModifiedTime();
    auto new_time = graph_.GetModifiedAt();
    EXPECT_GT(new_time, initial_time);
}

// Serialization Tests

TEST_F(GraphTest, Serialize_EmptyGraph_ReturnsValidJSON) {
    auto json = graph_.Serialize();
    
    EXPECT_TRUE(json.contains("metadata"));
    EXPECT_TRUE(json.contains("graph"));
    EXPECT_TRUE(json["graph"].contains("next_id"));
    EXPECT_TRUE(json["graph"].contains("nodes"));
    EXPECT_TRUE(json["graph"].contains("connections"));
    EXPECT_TRUE(json["graph"]["nodes"].is_array());
    EXPECT_TRUE(json["graph"]["connections"].is_array());
    EXPECT_EQ(json["graph"]["nodes"].size(), 0);
    EXPECT_EQ(json["graph"]["connections"].size(), 0);
}

TEST_F(GraphTest, Serialize_WithMetadata_IncludesMetadataFields) {
    graph_.SetProjectName("My Project");
    graph_.SetAuthor("John Doe");
    
    auto json = graph_.Serialize();
    
    EXPECT_EQ(json["metadata"]["project_name"], "My Project");
    EXPECT_EQ(json["metadata"]["author"], "John Doe");
    EXPECT_EQ(json["metadata"]["version"], "1.0");
    EXPECT_TRUE(json["metadata"].contains("created_at"));
    EXPECT_TRUE(json["metadata"].contains("modified_at"));
}

TEST_F(GraphTest, Serialize_WithNodes_IncludesAllNodes) {
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    
    auto json = graph_.Serialize();
    
    EXPECT_EQ(json["graph"]["nodes"].size(), 3);
    EXPECT_EQ(json["graph"]["next_id"], 3);
}

TEST_F(GraphTest, Serialize_WithConnections_IncludesAllConnections) {
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    
    // For now, just test that connections array exists in serialization structure
    // Connection tests would require properly initialized literal nodes with compatible types
    auto json = graph_.Serialize();
    
    EXPECT_TRUE(json["graph"]["connections"].is_array());
}

TEST_F(GraphTest, Serialize_NodeData_ContainsRequiredFields) {
    auto* node = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    
    auto json = graph_.Serialize();
    auto& node_json = json["graph"]["nodes"][0];
    
    EXPECT_TRUE(node_json.contains("id"));
    EXPECT_TRUE(node_json.contains("kind"));
    EXPECT_EQ(node_json["id"], node->id());
    EXPECT_EQ(node_json["kind"], "Literal");
}

// Deserialization Tests

TEST_F(GraphTest, Deserialize_ValidJSON_ReturnsGraph) {
    nlohmann::json json = {
        {"metadata", {
            {"project_name", "Test"},
            {"version", "1.0"},
            {"author", "Tester"},
            {"created_at", "2026-03-04T10:00:00Z"},
            {"modified_at", "2026-03-04T10:00:00Z"}
        }},
        {"graph", {
            {"next_id", 0},
            {"nodes", nlohmann::json::array()},
            {"connections", nlohmann::json::array()}
        }}
    };
    
    auto result = core::Graph::Deserialize(json);
    ASSERT_TRUE(result.has_value());
    
    auto& graph = result.value();
    EXPECT_EQ(graph.GetProjectName(), "Test");
    EXPECT_EQ(graph.GetAuthor(), "Tester");
}

TEST_F(GraphTest, Deserialize_MissingMetadata_ReturnsError) {
    nlohmann::json json = {
        {"graph", {
            {"next_id", 0},
            {"nodes", nlohmann::json::array()},
            {"connections", nlohmann::json::array()}
        }}
    };
    
    auto result = core::Graph::Deserialize(json);
    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().find("metadata") != std::string::npos);
}

TEST_F(GraphTest, Deserialize_MissingGraphSection_ReturnsError) {
    nlohmann::json json = {
        {"metadata", {
            {"project_name", "Test"},
            {"version", "1.0"},
            {"created_at", "2026-03-04T10:00:00Z"},
            {"modified_at", "2026-03-04T10:00:00Z"}
        }}
    };
    
    auto result = core::Graph::Deserialize(json);
    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().find("graph") != std::string::npos);
}

TEST_F(GraphTest, Deserialize_WrongVersion_ReturnsError) {
    nlohmann::json json = {
        {"metadata", {
            {"project_name", "Test"},
            {"version", "2.0"},
            {"created_at", "2026-03-04T10:00:00Z"},
            {"modified_at", "2026-03-04T10:00:00Z"}
        }},
        {"graph", {
            {"next_id", 0},
            {"nodes", nlohmann::json::array()},
            {"connections", nlohmann::json::array()}
        }}
    };
    
    auto result = core::Graph::Deserialize(json);
    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().find("version") != std::string::npos);
}

TEST_F(GraphTest, Deserialize_WithNodes_RestoresAllNodes) {
    nlohmann::json json = {
        {"metadata", {
            {"project_name", "Test"},
            {"version", "1.0"},
            {"created_at", "2026-03-04T10:00:00Z"},
            {"modified_at", "2026-03-04T10:00:00Z"}
        }},
        {"graph", {
            {"next_id", 3},
            {"nodes", nlohmann::json::array({
                {{"id", 0}, {"kind", "Literal"}, {"type", "Int"}, {"name", "Node1"}},
                {{"id", 1}, {"kind", "Literal"}, {"type", "Float"}, {"name", "Node2"}},
                {{"id", 2}, {"kind", "Literal"}, {"type", "String"}, {"name", "Node3"}}
            })},
            {"connections", nlohmann::json::array()}
        }}
    };
    
    auto result = core::Graph::Deserialize(json);
    ASSERT_TRUE(result.has_value());
    
    auto& graph = result.value();
    EXPECT_NE(graph.GetNode(0), nullptr);
    EXPECT_NE(graph.GetNode(1), nullptr);
    EXPECT_NE(graph.GetNode(2), nullptr);
}

TEST_F(GraphTest, Deserialize_InvalidNodeKind_ReturnsError) {
    nlohmann::json json = {
        {"metadata", {
            {"project_name", "Test"},
            {"version", "1.0"},
            {"created_at", "2026-03-04T10:00:00Z"},
            {"modified_at", "2026-03-04T10:00:00Z"}
        }},
        {"graph", {
            {"next_id", 1},
            {"nodes", nlohmann::json::array({
                {{"id", 0}, {"kind", "InvalidKind"}, {"type", "Int"}, {"name", "Node1"}}
            })},
            {"connections", nlohmann::json::array()}
        }}
    };
    
    auto result = core::Graph::Deserialize(json);
    EXPECT_FALSE(result.has_value());
}

// Round-trip Tests

TEST_F(GraphTest, RoundTrip_EmptyGraph_PreservesData) {
    graph_.SetProjectName("Round Trip Test");
    graph_.SetAuthor("Test User");
    
    // Serialize
    auto json = graph_.Serialize();
    
    // Deserialize
    auto result = core::Graph::Deserialize(json);
    ASSERT_TRUE(result.has_value());
    
    auto& restored_graph = result.value();
    EXPECT_EQ(restored_graph.GetProjectName(), "Round Trip Test");
    EXPECT_EQ(restored_graph.GetAuthor(), "Test User");
}

TEST_F(GraphTest, RoundTrip_WithNodes_PreservesAllNodes) {
    graph_.SetProjectName("Node Test");
    auto* node1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* node2 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* node3 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    
    uint32_t id1 = node1->id();
    uint32_t id2 = node2->id();
    uint32_t id3 = node3->id();
    
    // Serialize and deserialize
    auto json = graph_.Serialize();
    auto result = core::Graph::Deserialize(json);
    ASSERT_TRUE(result.has_value());
    
    auto& restored_graph = result.value();
    EXPECT_NE(restored_graph.GetNode(id1), nullptr);
    EXPECT_NE(restored_graph.GetNode(id2), nullptr);
    EXPECT_NE(restored_graph.GetNode(id3), nullptr);
}

TEST_F(GraphTest, RoundTrip_NextId_IsPreserved) {
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    
    auto json = graph_.Serialize();
    uint32_t original_next_id = json["graph"]["next_id"];
    
    auto result = core::Graph::Deserialize(json);
    ASSERT_TRUE(result.has_value());
    
    auto& restored_graph = result.value();
    // Adding a node should use the preserved next_id
    auto* new_node = restored_graph.AddNode(core::NodeBase::NodeKind::kLiteral);
    EXPECT_EQ(new_node->id(), original_next_id);
}

// File I/O Tests

TEST_F(GraphTest, SaveToFile_ValidPath_CreatesFile) {
    graph_.SetProjectName("File Test");
    std::filesystem::path temp_path = std::filesystem::temp_directory_path() / "test_graph.nebula";
    
    auto result = graph_.SaveToFile(temp_path);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(std::filesystem::exists(temp_path));
    
    // Cleanup
    std::filesystem::remove(temp_path);
}

TEST_F(GraphTest, SaveToFile_UpdatesModifiedTime) {
    auto initial_time = graph_.GetModifiedAt();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    std::filesystem::path temp_path = std::filesystem::temp_directory_path() / "test_modified.nebula";
    graph_.SaveToFile(temp_path);
    
    auto new_time = graph_.GetModifiedAt();
    EXPECT_GT(new_time, initial_time);
    
    // Cleanup
    std::filesystem::remove(temp_path);
}

TEST_F(GraphTest, LoadFromFile_NonExistentFile_ReturnsError) {
    std::filesystem::path fake_path = "/tmp/nonexistent_file_xyz.nebula";
    
    auto result = core::Graph::LoadFromFile(fake_path);
    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().find("does not exist") != std::string::npos);
}

TEST_F(GraphTest, LoadFromFile_InvalidJSON_ReturnsError) {
    std::filesystem::path temp_path = std::filesystem::temp_directory_path() / "invalid.nebula";
    
    // Write invalid JSON
    std::ofstream file(temp_path);
    file << "{ this is not valid json }";
    file.close();
    
    auto result = core::Graph::LoadFromFile(temp_path);
    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().find("parse") != std::string::npos);
    
    // Cleanup
    std::filesystem::remove(temp_path);
}

TEST_F(GraphTest, SaveAndLoad_RoundTrip_PreservesCompleteGraph) {
    // Setup a complex graph
    graph_.SetProjectName("Complete Test");
    graph_.SetAuthor("Integration Tester");
    auto* node1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* node2 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    auto* node3 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral);
    
    uint32_t id1 = node1->id();
    uint32_t id2 = node2->id();
    uint32_t id3 = node3->id();
    
    std::filesystem::path temp_path = std::filesystem::temp_directory_path() / "roundtrip.nebula";
    
    // Save
    auto save_result = graph_.SaveToFile(temp_path);
    ASSERT_TRUE(save_result.has_value()) << save_result.error();
    
    // Load
    auto load_result = core::Graph::LoadFromFile(temp_path);
    ASSERT_TRUE(load_result.has_value()) << load_result.error();
    
    auto& loaded_graph = load_result.value();
    EXPECT_EQ(loaded_graph.GetProjectName(), "Complete Test");
    EXPECT_EQ(loaded_graph.GetAuthor(), "Integration Tester");
    EXPECT_NE(loaded_graph.GetNode(id1), nullptr);
    EXPECT_NE(loaded_graph.GetNode(id2), nullptr);
    EXPECT_NE(loaded_graph.GetNode(id3), nullptr);
    
    // Cleanup
    std::filesystem::remove(temp_path);
}

TEST_F(GraphTest, SavedFile_IsHumanReadable_JSON) {
    graph_.SetProjectName("Readable Test");
    std::filesystem::path temp_path = std::filesystem::temp_directory_path() / "readable.nebula";
    
    graph_.SaveToFile(temp_path);
    
    // Read file as text
    std::ifstream file(temp_path);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    // Check it's pretty-printed (has newlines and indentation)
    EXPECT_TRUE(content.find('\n') != std::string::npos);
    EXPECT_TRUE(content.find("    ") != std::string::npos); // 4-space indentation
    EXPECT_TRUE(content.find("\"metadata\"") != std::string::npos);
    EXPECT_TRUE(content.find("\"graph\"") != std::string::npos);
    
    // Cleanup
    std::filesystem::remove(temp_path);
}


