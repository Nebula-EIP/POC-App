#include "graph.hpp"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <format>
#include <map>
#include <sstream>

#include "nodes/literal_node.hpp"

core::Graph::Graph()
    : next_id_(0),
      project_name_("Untitled Project"),
      version_("1.0"),
      author_(""),
      created_at_(std::chrono::system_clock::now()),
      modified_at_(std::chrono::system_clock::now()) {
}

void core::Graph::SetProjectName(const std::string& name) {
    project_name_ = name;
    UpdateModifiedTime();
}

void core::Graph::SetAuthor(const std::string& author) {
    author_ = author;
    UpdateModifiedTime();
}

void core::Graph::UpdateModifiedTime() {
    modified_at_ = std::chrono::system_clock::now();
}

core::NodeBase *core::Graph::AddNode(NodeBase::NodeKind kind) {
    if (kind == NodeBase::NodeKind::kUndefined) {
        return nullptr;
    }

    uint32_t id = next_id_++;
    auto node = CreateNode(id, kind);

    if (!node) {
        return nullptr;
    }

    nodes_.push_back(std::move(node));
    return nodes_.back().get();
}

void core::Graph::RemoveNode(NodeBase *node) {
    if (!node) return;

    // Disconnect all connections
    for (uint8_t i = 0; i < node->GetInputPinCount(); i++) {
        auto parent_conn = node->parent(i);
        if (parent_conn && parent_conn->IsConnected()) {
            Unlink(parent_conn->node, parent_conn->pin, node, i);
        }
    }

    // Same for outputs
    for (uint8_t i = 0; i < node->GetOutputPinCount(); i++) {
        auto children = node->childrens(i);

        // Copy to avoid iterator invalidation
        std::vector<NodeBase::Connection *> children_copy(children.begin(),
                                                          children.end());
        for (auto *child_conn : children_copy) {
            Unlink(node, i, child_conn->node, child_conn->pin);
        }
    }

    // Remove from nodes vector
    auto it = std::find_if(
        nodes_.begin(), nodes_.end(),
        [node](const std::unique_ptr<NodeBase> &n) { return n.get() == node; });

    if (it != nodes_.end()) {
        nodes_.erase(it);
    }
}

core::NodeBase *core::Graph::GetNode(uint32_t id) const {
    auto it = std::find_if(nodes_.begin(), nodes_.end(),
                           [id](const std::unique_ptr<NodeBase> &node) {
                               return node->id() == id;
                           });

    return it != nodes_.end() ? it->get() : nullptr;
}

std::expected<void, std::string> core::Graph::Link(NodeBase *from,
                                                   uint8_t out_pin,
                                                   NodeBase *to,
                                                   uint8_t in_pin) {
    if (!from || !to) {
        return std::unexpected(std::format(
            "{} pointer is null", ((from == nullptr) ? "1st" : "2nd")));
    }

    if (out_pin >= from->GetOutputPinCount()) {
        return std::unexpected("Output pin out of bounds");
    }

    if (in_pin >= to->GetInputPinCount()) {
        return std::unexpected("Input pin out of bounds");
    }

    auto res = from->CanConnectTo(out_pin, to, in_pin);
    if (!res) {
        return std::unexpected("{}");
    }

    to->SetParent(in_pin, from, out_pin);
    from->AddChild(out_pin, to, in_pin);

    return {};
}

std::expected<void, std::string> core::Graph::Unlink(NodeBase *from,
                                                     uint8_t out_pin,
                                                     NodeBase *to,
                                                     uint8_t in_pin) {
    if (!from || !to) {
        return std::unexpected("Invalid node pointers");
    }

    to->ClearParent(in_pin);
    from->RemoveChild(out_pin, to, in_pin);

    return {};
}

std::unique_ptr<core::NodeBase> core::Graph::CreateNode(
    uint32_t id, NodeBase::NodeKind kind) {
    std::unique_ptr<NodeBase> node;

    switch (kind) {
        case NodeBase::NodeKind::kLiteral:
            node = std::unique_ptr<LiteralNode>(new LiteralNode(id, kind));
            break;

        case NodeBase::NodeKind::kVariable:

        case NodeBase::NodeKind::kOperator:

        case NodeBase::NodeKind::kFunction:

        case NodeBase::NodeKind::kFunctionInput:

        case NodeBase::NodeKind::kFunctionOutput:

        case NodeBase::NodeKind::kCondition:

        case NodeBase::NodeKind::kLoop:

        case NodeBase::NodeKind::kUndefined:
        default:
            return nullptr;
    }

    // Initialize connection vectors after construction
    if (node) {
        node->InitializeConnections();
    }

    return node;
}

nlohmann::json core::Graph::Serialize() const {
    nlohmann::json json;

    // Serialize metadata
    json["metadata"]["project_name"] = project_name_;
    json["metadata"]["version"] = version_;
    json["metadata"]["author"] = author_;

    // Convert timestamps to ISO 8601 strings
    auto to_iso8601 = [](const std::chrono::system_clock::time_point& tp) {
        auto time = std::chrono::system_clock::to_time_t(tp);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    };

    json["metadata"]["created_at"] = to_iso8601(created_at_);
    json["metadata"]["modified_at"] = to_iso8601(modified_at_);

    // Serialize graph data
    json["graph"]["next_id"] = next_id_;

    // Serialize nodes
    nlohmann::json nodes_array = nlohmann::json::array();
    for (const auto& node : nodes_) {
        nodes_array.push_back(node->Serialize());
    }
    json["graph"]["nodes"] = nodes_array;

    // Serialize connections
    nlohmann::json connections_array = nlohmann::json::array();
    for (const auto& source_node : nodes_) {
        // Iterate through all output pins
        for (uint8_t out_pin = 0; out_pin < source_node->GetOutputPinCount(); ++out_pin) {
            const auto& children = source_node->childrens(out_pin);
            // Iterate through all connections on this output pin
            for (const auto& conn : children) {
                if (conn && conn->IsConnected()) {
                    nlohmann::json connection;
                    connection["source_node_id"] = source_node->id();
                    connection["source_pin"] = out_pin;
                    connection["target_node_id"] = conn->node->id();
                    connection["target_pin"] = conn->pin;
                    connections_array.push_back(connection);
                }
            }
        }
    }
    json["graph"]["connections"] = connections_array;

    return json;
}

std::expected<core::Graph, std::string> core::Graph::Deserialize(const nlohmann::json& json) {
    // Validate JSON structure
    if (!json.contains("metadata") || !json.contains("graph")) {
        return std::unexpected("Missing required top-level sections: metadata or graph");
    }

    const auto& metadata = json["metadata"];
    const auto& graph_data = json["graph"];

    // Validate metadata fields
    if (!metadata.contains("project_name") || !metadata.contains("version")) {
        return std::unexpected("Missing required metadata fields");
    }

    // Validate version
    std::string version = metadata["version"].get<std::string>();
    if (version != "1.0") {
        return std::unexpected("Unsupported .nebula format version: " + version);
    }

    // Create new graph
    Graph graph;

    // Restore metadata
    try {
        graph.project_name_ = metadata["project_name"].get<std::string>();
        graph.version_ = metadata["version"].get<std::string>();
        
        if (metadata.contains("author")) {
            graph.author_ = metadata["author"].get<std::string>();
        }

        // Parse timestamps from ISO 8601 strings
        auto parse_iso8601 = [](const std::string& iso_str) -> std::chrono::system_clock::time_point {
            std::tm tm = {};
            std::istringstream ss(iso_str);
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
            auto time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
            return time;
        };

        if (metadata.contains("created_at")) {
            graph.created_at_ = parse_iso8601(metadata["created_at"].get<std::string>());
        }
        if (metadata.contains("modified_at")) {
            graph.modified_at_ = parse_iso8601(metadata["modified_at"].get<std::string>());
        }
    } catch (const std::exception& e) {
        return std::unexpected(std::string("Failed to parse metadata: ") + e.what());
    }

    // Validate graph structure
    if (!graph_data.contains("next_id") || !graph_data.contains("nodes") || 
        !graph_data.contains("connections")) {
        return std::unexpected("Missing required graph fields: next_id, nodes, or connections");
    }

    // Restore next_id
    try {
        graph.next_id_ = graph_data["next_id"].get<uint32_t>();
    } catch (const std::exception& e) {
        return std::unexpected(std::string("Failed to parse next_id: ") + e.what());
    }

    // Deserialize nodes
    const auto& nodes_array = graph_data["nodes"];
    if (!nodes_array.is_array()) {
        return std::unexpected("Field 'nodes' is not an array");
    }

    std::map<uint32_t, NodeBase*> id_to_node_map;
    
    for (const auto& node_json : nodes_array) {
        auto result = NodeBase::Deserialize(node_json, &graph);
        if (!result) {
            return std::unexpected("Failed to deserialize node: " + result.error());
        }

        auto& node_ptr = result.value();
        uint32_t node_id = node_ptr->id();
        id_to_node_map[node_id] = node_ptr.get();
        graph.nodes_.push_back(std::move(node_ptr));
    }

    // Restore connections
    const auto& connections_array = graph_data["connections"];
    if (!connections_array.is_array()) {
        return std::unexpected("Field 'connections' is not an array");
    }

    for (const auto& conn_json : connections_array) {
        if (!conn_json.contains("source_node_id") || !conn_json.contains("source_pin") ||
            !conn_json.contains("target_node_id") || !conn_json.contains("target_pin")) {
            return std::unexpected("Connection missing required fields");
        }

        try {
            uint32_t source_id = conn_json["source_node_id"].get<uint32_t>();
            uint8_t source_pin = conn_json["source_pin"].get<uint8_t>();
            uint32_t target_id = conn_json["target_node_id"].get<uint32_t>();
            uint8_t target_pin = conn_json["target_pin"].get<uint8_t>();

            // Find nodes by ID
            auto source_it = id_to_node_map.find(source_id);
            auto target_it = id_to_node_map.find(target_id);

            if (source_it == id_to_node_map.end()) {
                return std::unexpected("Connection references non-existent source node ID: " + std::to_string(source_id));
            }
            if (target_it == id_to_node_map.end()) {
                return std::unexpected("Connection references non-existent target node ID: " + std::to_string(target_id));
            }

            NodeBase* source = source_it->second;
            NodeBase* target = target_it->second;

            // Validate pin indices
            if (source_pin >= source->GetOutputPinCount()) {
                return std::unexpected("Source pin index out of bounds");
            }
            if (target_pin >= target->GetInputPinCount()) {
                return std::unexpected("Target pin index out of bounds");
            }

            // Validate pin type compatibility
            auto type_result = source->CanConnectTo(source_pin, target, target_pin);
            if (!type_result) {
                return std::unexpected("Nodes cannot be connected: " + type_result.error());
            }

            // Establish the connection
            target->SetParent(target_pin, source, source_pin);
            source->AddChild(source_pin, target, target_pin);
        } catch (const std::exception& e) {
            return std::unexpected(std::string("Failed to parse connection: ") + e.what());
        }
    }

    return graph;
}
