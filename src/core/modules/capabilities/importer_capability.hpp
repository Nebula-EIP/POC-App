/**
 * @file importer_capability.hpp
 * @brief Defines the IImporterCapability interface for modules
 *
 * that can import source code into a graph.
 *
 * @author Created by Nolan Papa
 * @date Created on 06-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 06-09-2026
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <string>
#include <string_view>

#include "../../exception/graph_exception/connection_exception.hpp"
#include "../../exception/graph_exception/node_exception.hpp"
#include "../../graph/graph.hpp"
#include "../icapability.hpp"

namespace core::capa {

/**
 * @brief Entity associated with an import failure.
 */
enum class ImportEntityKind { kSource, kNode, kConnection };

/**
 * @brief Structured error returned by an importer capability.
 */
struct ImportError {
    ImportEntityKind kind_ = ImportEntityKind::kSource;
    uint32_t entity_id_ = 0;
    std::size_t source_offset_ = 0;
    std::string message_;
};

/**
 * @brief Input supplied to an importer capability.
 *
 * The target graph is owned by the caller. Implementations may report graph
 * invariant violations by returning ImportError or by propagating the graph
 * exceptions defined by the core, such as InvalidNodeException or
 * InvalidConnectionException.
 */
struct ImportRequest {
    Graph &graph_;
    std::string_view source_;
};

/**
 * @brief Optional module capability that turns written source code into graph
 * data.
 *
 * A module owns the source grammar it understands. The core only supplies the
 * target graph and routes the request to the selected module capability.
 */
class IImporterCapability : public core::ICapability {
   public:
    ~IImporterCapability() override = default;

    /**
     * @brief Imports source code into the requested graph.
     *
     * @param request Source text and target graph. Required.
     * @return Success, or a structured error identifying the failed entity and
     * source position.
     */
    virtual std::expected<void, ImportError> ImportCode(
        const ImportRequest &request) const = 0;
};

/**
 * @brief Concrete importer adapter for module-owned parsers.
 *
 * The core does not define a source grammar. Modules provide the parser as a
 * handler and this capability forwards import requests to it.
 */
class ImporterCapability final : public IImporterCapability {
   public:
    using ImportHandler = std::function<std::expected<void, ImportError>(
        const ImportRequest &request)>;

    ImporterCapability() = default;
    explicit ImporterCapability(ImportHandler handler);
    ~ImporterCapability() override = default;

    /**
     * @brief Replaces the parser used to process import requests.
     *
     * @param handler Module-owned parser. An empty handler disables importing.
     */
    void SetImportHandler(ImportHandler handler);

    /**
     * @brief Checks whether a parser is configured.
     *
     * @return true when import requests can be delegated to a parser.
     */
    bool HasImportHandler() const noexcept;

    /**
     * @brief Imports source code into the requested graph.
     *
     * @param request Source text and target graph. Required.
     * @return Success, or a structured error identifying the failed entity and
     * source position.
     */
    std::expected<void, ImportError> ImportCode(
        const ImportRequest &request) const override;

   private:
    ImportHandler handler_;
};

}  // namespace core::capa