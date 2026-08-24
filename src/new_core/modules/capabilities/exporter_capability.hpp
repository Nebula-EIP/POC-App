/*
 * @file exporter_capability.hpp
 * @brief Defines the capability for a module to export its code.
 *
 * @author Created by nolan papa
 * @date Created on  11-08-2026
 *
 * @author Last modified by nolan papa
 * @date Last modified on 11-08-2026
 */
#pragma once

#include <expected>
#include <span>
#include <string>
#include <string_view>

#include "../../graph/graph.hpp"
#include "../../graph/node.hpp"
#include "../icapability.hpp"
#include "node_list_capability.hpp"
#include "type_list_capability.hpp"

namespace code_generation {
class CodeGeneratorFile;
}

namespace core::capa {

/**
 * @brief Minimal type descriptor needed by the exporter capability.
 */
struct TypeDescriptor {
    DataType id = 0;
    std::string_view name;
};

/**
 * @brief Minimal node descriptor needed by the exporter capability.
 */
struct NodeDescriptor {
    NodeType id = 0;
    std::string_view name;
};

/**
 * @brief Optional capability used by the graph exporter to emit module-owned
 * source code.
 *
 * The exporter can call implementations in this order:
 * 1) exportPreamble(): module-wide includes/helpers (at most once)
 * 2) exportType(): support code for each module type used by the graph
 * 3) exportNode(): executable code for each node instance
 *
 * This capability is loaded after mandatory type/node list capabilities and
 * can be called in any module load order through ExportContext.
 */

enum class ExportEntityKind { kType, kNode };

/**
 * @brief Typed error returned by exporter capability operations.
 */
struct ExportError {
    ExportEntityKind kind = ExportEntityKind::kNode;
    uint32_t entity_id = 0;
    std::string message;
};

/**
 * @brief Shared context available to all exporter capability methods.
 *
 * Type and node lists are passed directly to keep this capability independent
 * from module loading order.
 */
struct ExportContext {
    /// Mandatory type metadata capability of the owning module.
    const ITypeListCapability &types;
    /// Mandatory node metadata capability of the owning module.
    const INodeListCapability &nodes;
    /// Graph being exported.
    const Graph &graph;
};

/**
 * @brief Minimum data required to compile one node into source code.
 */
struct NodeExportRequest {
    /// Runtime node instance to export.
    const Node &node;
    /// Static descriptor of the node type.
    const NodeDescriptor &descriptor;

    /**
     * One expression per declared input pin (same index order as
     * NodeDescriptor::input_pins). Empty values represent disconnected inputs.
     */
    std::span<const std::string_view> input_expressions;

    /**
     * One output symbol per declared output pin (same index order as
     * NodeDescriptor::output_pins).
     */
    std::span<const std::string_view> output_symbols;
};

/**
 * @brief Optional module capability generating source code for module
 * declared types and nodes.
 */
class IExporterCapability : public core::ICapability {
   public:
    ~IExporterCapability() override = default;

    /**
     * @brief Emits support code for one module type.
     *
     * @param out The code generator file to write to. Required.
     * @param context The shared export context. Required.
     * @param type The type descriptor to export. Required.
     * @return std::expected<void, ExportError> indicating success or failure.
     * Implementations should emit declarations/helpers that are required for
     * nodes using this type to compile.
     */
    virtual std::expected<void, ExportError> exportType(
        code_generation::CodeGeneratorFile &out, const ExportContext &context,
        const TypeDescriptor &type) const = 0;

    /**
     * @brief Emits executable code for one module node instance.
     *
     * @param out The code generator file to write to. Required.
     * @param context The shared export context. Required.
     * @param request The node export request containing the node instance,
     * @return std::expected<void, ExportError> indicating success or failure.
     * input_expressions and output_symbols are positional and must match
     * NodeDescriptor pin ordering.
     */
    virtual std::expected<void, ExportError> exportNode(
        code_generation::CodeGeneratorFile &out, const ExportContext &context,
        const NodeExportRequest &request) const = 0;

    /**
     * @brief Optional hook to emit module-level includes or helpers once.
     *
     * @param out The code generator file to write to. Required.
     * @param context The shared export context. Required.
     * @return std::expected<void, ExportError> indicating success or failure.
     * Implementations should emit any module-wide includes, helpers, or
     * declarations that are required for the exported nodes to compile. This
     * method is called at most once per module during the export process.
     */
    virtual std::expected<void, ExportError> exportPreamble(
        code_generation::CodeGeneratorFile &, const ExportContext &) const {
        return {};
    }
};

}  // namespace core::capa