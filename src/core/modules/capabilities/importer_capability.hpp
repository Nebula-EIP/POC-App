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
#include <string>
#include <string_view>

#include "../../graph/graph.hpp"
#include "../../exception/graph_exception/connection_exception.hpp"
#include "../../exception/graph_exception/node_exception.hpp"
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
	ImportEntityKind kind = ImportEntityKind::kSource;
	uint32_t entity_id = 0;
	std::size_t source_offset = 0;
	std::string message;
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
	Graph &graph;
	std::string_view source;
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
	virtual std::expected<void, ImportError> importCode(
		const ImportRequest &request) const = 0;
};

}  // namespace core::capa