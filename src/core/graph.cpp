#include "graph.hpp"

#include <algorithm>
#include <ctime>
#include <format>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>

#include "nodes/function_input_node.hpp"
#include "nodes/function_node.hpp"
#include "nodes/function_output_node.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/variable_node.hpp"

core::Graph::Graph()
    : project_name_("Untitled Project"),
      version_("1.0"),
      author_(""),
      created_at_(std::chrono::system_clock::now()),
      modified_at_(std::chrono::system_clock::now()) {}

void core::Graph::SetProjectName(const std::string &name) {
    project_name_ = name;
    UpdateModifiedTime();
}

void core::Graph::SetAuthor(const std::string &author) {
    author_ = author;
    UpdateModifiedTime();
}

void core::Graph::UpdateModifiedTime() {
    modified_at_ = std::chrono::system_clock::now();
}

const std::string &core::Graph::GetProjectName() const { return project_name_; }

const std::string &core::Graph::GetVersion() const { return version_; }

const std::string &core::Graph::GetAuthor() const { return author_; }

std::chrono::system_clock::time_point core::Graph::GetCreatedAt() const {
    return created_at_;
}

std::chrono::system_clock::time_point core::Graph::GetModifiedAt() const {
    return modified_at_;
}

core::NodeBase *core::Graph::AddNode(NodeBase::NodeKind kind) {
    if (kind == NodeBase::NodeKind::kUndefined) {
        THROW_EXCEPTION(InvalidNodeKindException, "kUndefined is not a valid node kind");
    }

    try {
        nodes_.push_back(CreateNode(id_manager_.NewId(), kind));
    } catch (std::exception &e) {
        return nullptr;
    }
    return nodes_.back().get();
}

void core::Graph::RemoveNode(NodeBase *node) {
    if (node == nullptr) {
        THROW_EXCEPTION(NodeNotFoundException, "node is nullptr");
    }

    auto it = std::find_if(
        nodes_.begin(), nodes_.end(),
        [node](const std::unique_ptr<NodeBase> &n) { return n->id() == node->id(); });
    
    if (it == nodes_.end()) {
        THROW_EXCEPTION(NodeNotFoundException, "node is not owned by this Graph");
    }

    // Disconnect all parents
    for (const NodeBase::Connection &conn : node->GetAllParents() ) {
        if (conn.IsConnected()) {
            Unlink(conn.node, conn.out_pin, node, conn.in_pin);
        }
    }

    // Disconnect all childrens
    for (const NodeBase::Connection &conn : node->GetAllChildrens()) {
        if (conn.IsConnected()) {
            Unlink(node, conn.out_pin, conn.node, conn.in_pin);
        }
    }

    // Free the id
    id_manager_.FreeId(node->id());

    // Delete the node
    nodes_.erase(it);
}

core::NodeBase *core::Graph::GetNode(uint32_t id) const {
    auto it = std::find_if(nodes_.begin(), nodes_.end(),
                           [id](const std::unique_ptr<NodeBase> &node) {
                               return node->id() == id;
                           });

    return it != nodes_.end() ? it->get() : nullptr;
}

const std::vector<std::unique_ptr<core::NodeBase>> &core::Graph::GetAllNodes() const noexcept {
    return nodes_;
}

void core::Graph::Link(NodeBase *from,
                       uint8_t out_pin,
                       NodeBase *to,
                       uint8_t in_pin) {
    if (!from || !to) {
        THROW_EXCEPTION(NodeNotFoundException, "{} node pointer is null",
            ((from == nullptr) ? "1st" : "2nd"));
    }

    // Check if nodes are in the local array
    auto from_it = std::find_if(
        nodes_.begin(), nodes_.end(),
        [from](const std::unique_ptr<NodeBase> &n) { return n->id() == from->id(); });
    
    if (from_it == nodes_.end()) {
        THROW_EXCEPTION(NodeNotFoundException, "from node is not owned by this Graph");
    }

    // Check if nodes are in the local array
    auto to_it = std::find_if(
        nodes_.begin(), nodes_.end(),
        [to](const std::unique_ptr<NodeBase> &n) { return n->id() == to->id(); });
    
    if (to_it == nodes_.end()) {
        THROW_EXCEPTION(NodeNotFoundException, "to node is not owned by this Graph");
    }

    // Check if pins exists
    if (!from->OutputPinExists(out_pin)) {
        THROW_EXCEPTION(InvalidPinIndexException, "from node has no output pin with an id of {}", out_pin);
    }

    if (!to->InputPinExists(in_pin)) {
        THROW_EXCEPTION(InvalidPinIndexException, "to node has no input pin with an id of {}", in_pin);
    }

    // Check that types matches
    auto from_type = from->GetOutputPinType(out_pin);
    auto to_type = to->GetInputPinType(in_pin);

    if (from_type != to_type) {
        THROW_EXCEPTION(IncompatiblePinTypesException, "trying to connect in({}) to out({})",
            PinDataTypeToString(from_type), PinDataTypeToString(to_type));
    }

    // Check for circular dependency
    if (from == to) {
        THROW_EXCEPTION(CircularDependencyException, "cannot link a node to itself");
    }

    // Severe previous links if required
    if (to->IsInputPinConnected(in_pin)) {
        auto conn = to->parent(in_pin);
        Unlink(conn.node, conn.out_pin, to, conn.in_pin);
    }

    // Link !
    to->SetParent(in_pin, from, out_pin);
    from->AddChild(out_pin, to, in_pin);
}

void core::Graph::Unlink(NodeBase *from,
                         uint8_t out_pin,
                         NodeBase *to,
                         uint8_t in_pin) {
    if (!from || !to) {
        THROW_EXCEPTION(NodeNotFoundException, "{} node pointer is null",
            ((from == nullptr) ? "1st" : "2nd"));
    }

    // Check if nodes are in the local array
    auto from_it = std::find_if(
        nodes_.begin(), nodes_.end(),
        [from](const std::unique_ptr<NodeBase> &n) { return n->id() == from->id(); });
    
    if (from_it == nodes_.end()) {
        THROW_EXCEPTION(NodeNotFoundException, "from node is not owned by this Graph");
    }

    // Check if nodes are in the local array
    auto to_it = std::find_if(
        nodes_.begin(), nodes_.end(),
        [to](const std::unique_ptr<NodeBase> &n) { return n->id() == to->id(); });
    
    if (to_it == nodes_.end()) {
        THROW_EXCEPTION(NodeNotFoundException, "to node is not owned by this Graph");
    }

    // Check if pins exists
    if (!from->OutputPinExists(out_pin)) {
        THROW_EXCEPTION(InvalidPinIndexException, "from node has no output pin with an id of {}", out_pin);
    }

    if (!to->InputPinExists(in_pin)) {
        THROW_EXCEPTION(InvalidPinIndexException, "to node has no input pin with an id of {}", in_pin);
    }

    to->ClearParent(in_pin);
    from->RemoveChild(out_pin, to, in_pin);
}

std::unique_ptr<core::NodeBase> core::Graph::CreateNode(
    uint32_t id, NodeBase::NodeKind kind) {
    std::unique_ptr<NodeBase> node;

    switch (kind) {
        case NodeBase::NodeKind::kLiteral:
            node = std::unique_ptr<LiteralNode>(new LiteralNode(id, kind));
            break;

        case NodeBase::NodeKind::kVariable:
            node = std::unique_ptr<VariableNode>(new VariableNode(id, kind));
            break;

        case NodeBase::NodeKind::kFunction:
            node = std::unique_ptr<FunctionNode>(new FunctionNode(id, kind));
            break;

        case NodeBase::NodeKind::kFunctionInput:
            node = std::unique_ptr<FunctionInputNode>(
                new FunctionInputNode(id, kind));
            break;

        case NodeBase::NodeKind::kFunctionOutput:
            node = std::unique_ptr<FunctionOutputNode>(
                new FunctionOutputNode(id, kind));
            break;

        case NodeBase::NodeKind::kOperator:

        case NodeBase::NodeKind::kCondition:

        case NodeBase::NodeKind::kLoop:

        case NodeBase::NodeKind::kUndefined:
        default:
            return nullptr;
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
    auto to_iso8601 = [](const std::chrono::system_clock::time_point &tp) {
        auto time = std::chrono::system_clock::to_time_t(tp);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    };

    json["metadata"]["created_at"] = to_iso8601(created_at_);
    json["metadata"]["modified_at"] = to_iso8601(modified_at_);

    // Serialize graph data
    json["graph"]["next_id"] = id_manager_.current_id();

    // Serialize nodes
    nlohmann::json nodes_array = nlohmann::json::array();
    for (const auto &node : nodes_) {
        nodes_array.push_back(node->Serialize());
    }
    json["graph"]["nodes"] = nodes_array;

    // Serialize connections
    nlohmann::json connections_array = nlohmann::json::array();
    for (const auto &source_node : nodes_) {
        // Iterate through all output pins
        for (uint8_t out_pin = 0; out_pin < source_node->GetOutputPinCount();
             ++out_pin) {
            auto children = source_node->childrens(out_pin);
            // Iterate through all connections on this output pin
            for (const auto &conn : (*children)) {
                if (conn.IsConnected()) {
                    nlohmann::json connection;
                    connection["source_node_id"] = source_node->id();
                    connection["source_pin"] = conn.out_pin;
                    connection["target_node_id"] = conn.node->id();
                    connection["target_pin"] = conn.in_pin;
                    connections_array.push_back(connection);
                }
            }
        }
    }
    json["graph"]["connections"] = connections_array;

    return json;
}

std::expected<core::Graph, std::string> core::Graph::Deserialize(
    const nlohmann::json &json) {
    // Validate JSON structure
    if (!json.contains("metadata") || !json.contains("graph")) {
        return std::unexpected(
            "Missing required top-level sections: metadata or graph");
    }

    const auto &metadata = json["metadata"];
    const auto &graph_data = json["graph"];

    // Validate metadata fields
    if (!metadata.contains("project_name") || !metadata.contains("version")) {
        return std::unexpected("Missing required metadata fields");
    }

    // Validate version
    std::string version = metadata["version"].get<std::string>();
    if (version != "1.0") {
        return std::unexpected("Unsupported .nebula format version: " +
                               version);
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
        auto parse_iso8601 = [](const std::string &iso_str)
            -> std::chrono::system_clock::time_point {
            std::tm tm = {};
            std::istringstream ss(iso_str);
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
            auto time =
                std::chrono::system_clock::from_time_t(std::mktime(&tm));
            return time;
        };

        if (metadata.contains("created_at")) {
            graph.created_at_ =
                parse_iso8601(metadata["created_at"].get<std::string>());
        }
        if (metadata.contains("modified_at")) {
            graph.modified_at_ =
                parse_iso8601(metadata["modified_at"].get<std::string>());
        }
    } catch (const std::exception &e) {
        return std::unexpected(std::string("Failed to parse metadata: ") +
                               e.what());
    }

    // Validate graph structure
    if (!graph_data.contains("next_id") || !graph_data.contains("nodes") ||
        !graph_data.contains("connections")) {
        return std::unexpected(
            "Missing required graph fields: next_id, nodes, or connections");
    }

    // Restore next_id
    try {
        graph.id_manager_ = utils::IdManager<uint32_t>(graph_data["next_id"].get<uint32_t>());
    } catch (const std::exception &e) {
        return std::unexpected(std::string("Failed to parse next_id: ") +
                               e.what());
    }

    // Deserialize nodes
    const auto &nodes_array = graph_data["nodes"];
    if (!nodes_array.is_array()) {
        return std::unexpected("Field 'nodes' is not an array");
    }

    std::map<uint32_t, NodeBase *> id_to_node_map;

    for (const auto &node_json : nodes_array) {
        auto result = NodeBase::DeserializeFactory(node_json, &graph);
        if (!result) {
            return std::unexpected("Failed to deserialize node: " +
                                   result.error());
        }

        auto &node_ptr = result.value();
        uint32_t node_id = node_ptr->id();
        id_to_node_map[node_id] = node_ptr.get();
        graph.nodes_.push_back(std::move(node_ptr));
    }

    // Restore connections
    const auto &connections_array = graph_data["connections"];
    if (!connections_array.is_array()) {
        return std::unexpected("Field 'connections' is not an array");
    }

    for (const auto &conn_json : connections_array) {
        if (!conn_json.contains("source_node_id") ||
            !conn_json.contains("source_pin") ||
            !conn_json.contains("target_node_id") ||
            !conn_json.contains("target_pin")) {
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
                return std::unexpected(
                    "Connection references non-existent source node ID: " +
                    std::to_string(source_id));
            }
            if (target_it == id_to_node_map.end()) {
                return std::unexpected(
                    "Connection references non-existent target node ID: " +
                    std::to_string(target_id));
            }

            NodeBase *source = source_it->second;
            NodeBase *target = target_it->second;

            // Validate pin indices
            if (source_pin >= source->GetOutputPinCount()) {
                return std::unexpected("Source pin index out of bounds");
            }
            if (target_pin >= target->GetInputPinCount()) {
                return std::unexpected("Target pin index out of bounds");
            }

            // Validate pin type compatibility
            auto type_result =
                source->CanConnectTo(source_pin, target, target_pin);
            if (!type_result) {
                return std::unexpected("Nodes cannot be connected: " +
                                       type_result.error());
            }

            // Establish the connection
            target->SetParent(target_pin, source, source_pin);
            source->AddChild(source_pin, target, target_pin);
        } catch (const std::exception &e) {
            return std::unexpected(std::string("Failed to parse connection: ") +
                                   e.what());
        }
    }

    return graph;
}

std::expected<void, std::string> core::Graph::SaveToFile(
    const std::filesystem::path &path) {
    try {
        // Update modified timestamp before saving
        UpdateModifiedTime();

        // Serialize the graph
        nlohmann::json json = Serialize();

        // Write to file with pretty printing
        std::ofstream file(path);
        if (!file.is_open()) {
            return std::unexpected("Failed to open file for writing: " +
                                   path.string());
        }

        file << json.dump(4) << std::endl;
        file.close();

        return {};
    } catch (const std::filesystem::filesystem_error &e) {
        return std::unexpected(std::string("Filesystem error: ") + e.what());
    } catch (const std::exception &e) {
        return std::unexpected(std::string("Failed to save graph: ") +
                               e.what());
    }
}

std::expected<core::Graph, std::string> core::Graph::LoadFromFile(
    const std::filesystem::path &path) {
    try {
        // Check if file exists
        if (!std::filesystem::exists(path)) {
            return std::unexpected("File does not exist: " + path.string());
        }

        // Read file
        std::ifstream file(path);
        if (!file.is_open()) {
            return std::unexpected("Failed to open file for reading: " +
                                   path.string());
        }

        // Parse JSON
        nlohmann::json json;
        try {
            file >> json;
        } catch (const nlohmann::json::parse_error &e) {
            return std::unexpected(std::string("Failed to parse JSON: ") +
                                   e.what());
        }

        file.close();

        // Deserialize the graph
        return Deserialize(json);
    } catch (const std::filesystem::filesystem_error &e) {
        return std::unexpected(std::string("Filesystem error: ") + e.what());
    } catch (const std::exception &e) {
        return std::unexpected(std::string("Failed to load graph: ") +
                               e.what());
    }
}
