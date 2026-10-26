#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "graph/datatypes.hpp"

namespace editor::project {

inline constexpr const char *kNebulaVersion = "1.1";

/// Thrown for any unreadable, invalid or unwritable .nebula file.
class NebulaFileError : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
};

struct NebulaMetadata {
    std::string project_name_ = "Untitled";
    std::string author_;
    std::string created_at_;
    std::string modified_at_;
};

struct NebulaPin {
    core::PinId id_ = 0;
    std::string name_;
    core::DataType type_ = 0;
};

struct NebulaNode {
    core::NodeId id_ = 0;
    core::NodeType type_ = 0;
    std::string type_name_;  ///< Catalog name, checked on load.
    std::string title_;
    float x_ = 0.0F;
    float y_ = 0.0F;
    std::vector<NebulaPin> inputs_;
    std::vector<NebulaPin> outputs_;
    core::PropertyMap properties_;
};

struct NebulaConnection {
    core::NodeId source_node_id_ = 0;
    core::PinId source_pin_ = 0;
    core::NodeId target_node_id_ = 0;
    core::PinId target_pin_ = 0;
};

struct NebulaDocument {
    NebulaMetadata metadata_;
    std::vector<NebulaNode> nodes_;
    std::vector<NebulaConnection> connections_;
};

/// @throws NebulaFileError when a property holds an unsupported type.
[[nodiscard]] std::string ToJsonString(const NebulaDocument &document);

/// @throws NebulaFileError on invalid JSON or any violated validation rule.
[[nodiscard]] NebulaDocument FromJsonString(const std::string &text);

/// Writes through a temporary file, then renames it over the destination.
void WriteNebulaFile(const std::filesystem::path &path,
                     const NebulaDocument &document);

[[nodiscard]] NebulaDocument ReadNebulaFile(const std::filesystem::path &path);

/// Current UTC time, ISO 8601 (e.g. 2026-10-25T10:30:00Z).
[[nodiscard]] std::string NowIso8601();

}  // namespace editor::project