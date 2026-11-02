#include "project/nebula_file.hpp"

#include <algorithm>
#include <any>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace editor::project {

namespace {
using nlohmann::json;

// ---- std::any <-> JSON ------------------------------------------------

template <typename T>
bool Encode(const std::any &value, const char *name, std::string &kind,
            json &out) {
    if (const T *typed = std::any_cast<T>(&value)) {
        kind = name;
        out = *typed;
        return true;
    }
    return false;
}

bool EncodeAny(const std::any &value, std::string &kind, json &out) {
    if (!value.has_value()) {
        kind = "empty";
        out = nullptr;
        return true;
    }
    return Encode<bool>(value, "bool", kind, out) ||
           Encode<int>(value, "int", kind, out) ||
           Encode<unsigned int>(value, "uint", kind, out) ||
           Encode<std::int64_t>(value, "int64", kind, out) ||
           Encode<std::uint64_t>(value, "uint64", kind, out) ||
           Encode<float>(value, "float", kind, out) ||
           Encode<double>(value, "double", kind, out) ||
           Encode<std::string>(value, "string", kind, out);
}

template <typename T>
bool Decode(const std::string &kind, const char *name, const json &in,
            std::any &out) {
    if (kind != name) {
        return false;
    }
    out = in.get<T>();
    return true;
}

std::any DecodeAny(const std::string &kind, const json &in) {
    std::any out;
    if (kind == "empty") {
        return out;
    }
    if (Decode<bool>(kind, "bool", in, out) ||
        Decode<int>(kind, "int", in, out) ||
        Decode<unsigned int>(kind, "uint", in, out) ||
        Decode<std::int64_t>(kind, "int64", in, out) ||
        Decode<std::uint64_t>(kind, "uint64", in, out) ||
        Decode<float>(kind, "float", in, out) ||
        Decode<double>(kind, "double", in, out) ||
        Decode<std::string>(kind, "string", in, out)) {
        return out;
    }
    throw NebulaFileError("Unknown property kind '" + kind + "'");
}

// ---- Document <-> JSON --------------------------------------------------

json PinsToJson(const std::vector<NebulaPin> &pins) {
    json array = json::array();
    for (const NebulaPin &pin : pins) {
        array.push_back(
            {{"id", pin.id_}, {"name", pin.name_}, {"type", pin.type_}});
    }
    return array;
}

json NodeToJson(const NebulaNode &node) {
    json properties = json::array();
    // Sorted by id: the output must be deterministic (dirty detection).
    std::vector<core::PropertyId> ids;
    ids.reserve(node.properties_.size());
    for (const auto &[id, property] : node.properties_) {
        (void)property;
        ids.push_back(id);
    }
    std::sort(ids.begin(), ids.end());
    for (const core::PropertyId kId : ids) {
        const core::Property &property = node.properties_.at(kId);
        std::string kind;
        json value;
        if (!EncodeAny(property.value_, kind, value)) {
            throw NebulaFileError("Node " + std::to_string(node.id_) +
                                  ", property " + std::to_string(kId) +
                                  ": unsupported value type '" +
                                  property.value_.type().name() + "'");
        }
        properties.push_back({{"id", kId},
                              {"type_id", property.type_id_},
                              {"kind", kind},
                              {"value", value}});
    }
    return {{"id", node.id_},
            {"type", node.type_},
            {"type_name", node.type_name_},
            {"title", node.title_},
            {"x", node.x_},
            {"y", node.y_},
            {"inputs", PinsToJson(node.inputs_)},
            {"outputs", PinsToJson(node.outputs_)},
            {"properties", properties}};
}

std::vector<NebulaPin> PinsFromJson(const json &array) {
    std::vector<NebulaPin> pins;
    for (const json &item : array) {
        pins.push_back({item.at("id").get<core::PinId>(),
                        item.at("name").get<std::string>(),
                        item.at("type").get<core::DataType>()});
    }
    return pins;
}

NebulaNode NodeFromJson(const json &item) {
    NebulaNode node;
    node.id_ = item.at("id").get<core::NodeId>();
    node.type_ = item.at("type").get<core::NodeType>();
    node.type_name_ = item.value("type_name", std::string{});
    node.title_ = item.value("title", std::string{});
    node.x_ = item.value("x", 0.0F);
    node.y_ = item.value("y", 0.0F);
    node.inputs_ = PinsFromJson(item.at("inputs"));
    node.outputs_ = PinsFromJson(item.at("outputs"));
    for (const json &property : item.at("properties")) {
        const auto kId = property.at("id").get<core::PropertyId>();
        if (node.properties_.contains(kId)) {
            throw NebulaFileError("Node " + std::to_string(node.id_) +
                                  ": duplicate property id " +
                                  std::to_string(kId));
        }
        node.properties_[kId] =
            core::Property{property.at("type_id").get<core::PropertyTypeId>(),
                           DecodeAny(property.at("kind").get<std::string>(),
                                     property.at("value"))};
    }
    return node;
}

void Validate(const NebulaDocument &document, std::uint64_t next_id) {
    std::unordered_map<core::NodeId, const NebulaNode *> by_id;
    for (const NebulaNode &node : document.nodes_) {
        if (!by_id.emplace(node.id_, &node).second) {
            throw NebulaFileError("Duplicate node id " +
                                  std::to_string(node.id_));
        }
        if (node.id_ >= next_id) {
            throw NebulaFileError("next_id (" + std::to_string(next_id) +
                                  ") must be greater than every node id");
        }
        std::set<core::PinId> seen;
        for (const auto *pins : {&node.inputs_, &node.outputs_}) {
            seen.clear();
            for (const NebulaPin &pin : *pins) {
                if (!seen.insert(pin.id_).second) {
                    throw NebulaFileError("Node " + std::to_string(node.id_) +
                                          ": duplicate pin id " +
                                          std::to_string(pin.id_));
                }
            }
        }
    }

    const auto kHasPin = [](const std::vector<NebulaPin> &pins,
                            core::PinId id) {
        return std::any_of(pins.begin(), pins.end(),
                           [id](const NebulaPin &p) { return p.id_ == id; });
    };
    for (const NebulaConnection &c : document.connections_) {
        const auto kFrom = by_id.find(c.source_node_id_);
        const auto kTo = by_id.find(c.target_node_id_);
        if (kFrom == by_id.end() || kTo == by_id.end()) {
            throw NebulaFileError("Dangling connection: node " +
                                  std::to_string(kFrom == by_id.end()
                                                     ? c.source_node_id_
                                                     : c.target_node_id_) +
                                  " does not exist");
        }
        if (!kHasPin(kFrom->second->outputs_, c.source_pin_) ||
            !kHasPin(kTo->second->inputs_, c.target_pin_)) {
            throw NebulaFileError("Invalid pin in connection " +
                                  std::to_string(c.source_node_id_) + ":" +
                                  std::to_string(c.source_pin_) + " -> " +
                                  std::to_string(c.target_node_id_) + ":" +
                                  std::to_string(c.target_pin_));
        }
    }
}
}  // namespace

std::string NowIso8601() {
    const std::time_t kNow =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm utc{};
#if defined(_WIN32)
    gmtime_s(&utc, &kNow);
#else
    gmtime_r(&kNow, &utc);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utc);
    return buffer;
}

std::string ToJsonString(const NebulaDocument &document) {
    NebulaDocument sorted = document;
    std::sort(
        sorted.nodes_.begin(), sorted.nodes_.end(),
        [](const NebulaNode &a, const NebulaNode &b) { return a.id_ < b.id_; });
    std::sort(sorted.connections_.begin(), sorted.connections_.end(),
              [](const NebulaConnection &a, const NebulaConnection &b) {
                  return std::tie(a.source_node_id_, a.source_pin_,
                                  a.target_node_id_, a.target_pin_) <
                         std::tie(b.source_node_id_, b.source_pin_,
                                  b.target_node_id_, b.target_pin_);
              });

    json nodes = json::array();
    core::NodeId max_id = 0;
    for (const NebulaNode &node : sorted.nodes_) {
        nodes.push_back(NodeToJson(node));
        max_id = std::max(max_id, node.id_);
    }
    json connections = json::array();
    for (const NebulaConnection &c : sorted.connections_) {
        connections.push_back({{"source_node_id", c.source_node_id_},
                               {"source_pin", c.source_pin_},
                               {"target_node_id", c.target_node_id_},
                               {"target_pin", c.target_pin_}});
    }

    const NebulaMetadata &meta = sorted.metadata_;
    json root = {{"metadata",
                  {{"project_name", meta.project_name_},
                   {"version", kNebulaVersion},
                   {"author", meta.author_},
                   {"created_at", meta.created_at_},
                   {"modified_at", meta.modified_at_}}},
                 {"graph",
                  {{"next_id", sorted.nodes_.empty() ? 1U : max_id + 1U},
                   {"nodes", nodes},
                   {"connections", connections}}}};
    return root.dump(2);
}

NebulaDocument FromJsonString(const std::string &text) {
    json root;
    try {
        root = json::parse(text);
    } catch (const json::exception &error) {
        throw NebulaFileError(std::string("Invalid JSON: ") + error.what());
    }

    try {
        NebulaDocument document;
        const json &meta = root.at("metadata");
        const auto kVersion = meta.at("version").get<std::string>();
        if (kVersion != kNebulaVersion) {
            throw NebulaFileError("Unsupported version '" + kVersion +
                                  "' (expected " + kNebulaVersion + ")");
        }
        document.metadata_.project_name_ =
            meta.at("project_name").get<std::string>();
        document.metadata_.author_ = meta.value("author", std::string{});
        document.metadata_.created_at_ =
            meta.at("created_at").get<std::string>();
        document.metadata_.modified_at_ =
            meta.at("modified_at").get<std::string>();

        const json &graph = root.at("graph");
        const auto kNextId = graph.at("next_id").get<std::uint64_t>();
        for (const json &item : graph.at("nodes")) {
            document.nodes_.push_back(NodeFromJson(item));
        }
        for (const json &item : graph.at("connections")) {
            document.connections_.push_back(
                {item.at("source_node_id").get<core::NodeId>(),
                 item.at("source_pin").get<core::PinId>(),
                 item.at("target_node_id").get<core::NodeId>(),
                 item.at("target_pin").get<core::PinId>()});
        }
        Validate(document, kNextId);
        return document;
    } catch (const json::exception &error) {
        throw NebulaFileError(std::string("Invalid .nebula file: ") +
                              error.what());
    }
}

void WriteNebulaFile(const std::filesystem::path &path,
                     const NebulaDocument &document) {
    const std::string kText = ToJsonString(document);

    std::filesystem::path temporary = path;
    temporary += ".tmp";
    {
        std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw NebulaFileError("Cannot write '" + temporary.string() + "'");
        }
        out << kText;
        out.flush();
        if (!out) {
            throw NebulaFileError("Write failed for '" + temporary.string() +
                                  "'");
        }
    }
    std::error_code error;
    std::filesystem::rename(temporary, path, error);
    if (error) {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        throw NebulaFileError("Cannot replace '" + path.string() +
                              "': " + error.message());
    }
}

NebulaDocument ReadNebulaFile(const std::filesystem::path &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw NebulaFileError("Cannot open '" + path.string() + "'");
    }
    const std::string kText((std::istreambuf_iterator<char>(in)),
                            std::istreambuf_iterator<char>());
    return FromJsonString(kText);
}

}  // namespace editor::project