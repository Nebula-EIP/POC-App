/**
 * @file importer_capability.cpp
 * @brief Implementation of the handler-based importer capability.
 *
 * @author Created by Nolan Papa
 * @date Created on 13-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 13-09-2026
 */

#include "importer_capability.hpp"

#include <utility>

namespace core::capa {

ImporterCapability::ImporterCapability(ImportHandler handler)
    : handler_(std::move(handler)) {}

void ImporterCapability::setImportHandler(ImportHandler handler) {
    handler_ = std::move(handler);
}

bool ImporterCapability::hasImportHandler() const noexcept {
    return static_cast<bool>(handler_);
}

std::expected<void, ImportError> ImporterCapability::importCode(
    const ImportRequest &request) const {
    if (!handler_) {
        return std::unexpected(
            ImportError{ImportEntityKind::kSource, 0, 0,
                        "No importer handler is configured"});
    }

    return handler_(request);
}

}  // namespace core::capa
