/**
 * @file importer_capability.hpp
 * @brief Defines the capability used to provide source files, nodes, and types
 *        to a graph importer.
 *
 * This capability allows a module to provide the information required by a
 * graph importer to reconstruct a graph from source code. Implementations can
 * load individual source files, recursively load a repository, and expose
 * the node and type definitions known by the module.
 *
 * @author Created by NathanBezard
 * @date Created on 11-08-2026
 *
 * @author Last modified by NathanBezard
 * @date Last modified on 11-08-2026
 */

#pragma once

#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

#include "../../graph/datatypes.hpp"
#include "../icapability.hpp"

namespace core::capa {

/**
 * @brief Describes a type available to the graph importer.
 *
 * A type descriptor contains the identifier assigned to the type by the core
 * and its human-readable name.
 */
struct ImportTypeDescriptor {
    /** @brief Unique identifier of the type. */
    DataType id = 0;

    /** @brief Name used to identify the type in source code. */
    std::string_view name;
};

/**
 * @brief Describes a node available to the graph importer.
 *
 * A node descriptor contains the identifier assigned to the node type by the
 * core and its human-readable name.
 */
struct ImportNodeDescriptor {
    /** @brief Unique identifier of the node type. */
    NodeType id = 0;

    /** @brief Name used to identify the node type in source code. */
    std::string_view name;
};

/**
 * @brief Error returned by an importer capability operation.
 *
 * The error contains the path associated with the failed operation and a
 * human-readable description of the failure.
 */
struct ImportError {
    /** @brief File or repository path associated with the error. */
    std::filesystem::path path;

    /** @brief Description of the error. */
    std::string message;
};

/**
 * @brief Optional capability used to provide source and metadata to a graph
 * importer.
 *
 * Implementations of this capability can provide source files to the importer
 * and expose the node and type definitions required to reconstruct a graph.
 *
 * The importer may use the capability in the following order:
 * 1) Load a source file or repository.
 * 2) Retrieve the available node definitions.
 * 3) Retrieve the available type definitions.
 * 4) Use the provided information to construct the graph.
 */
class IImporterCapability : public core::ICapability {
   public:
    /**
     * @brief Virtual destructor for the importer capability.
     */
    ~IImporterCapability() override = default;

    /**
     * @brief Loads the contents of a single source file.
     *
     * The implementation is responsible for opening the specified file and
     * returning its contents as text. The importer can then parse the returned
     * source code.
     *
     * @param file_path Path to the source file to load.
     *
     * @return The contents of the file on success, or an ImportError if the
     * file cannot be opened or read.
     */
    virtual std::string loadFile(const std::filesystem::path &file_path) const = 0;

    /**
     * @brief Recursively loads source files from a repository.
     *
     * The implementation recursively traverses the specified repository and
     * loads the source files that are supported by the importer. Files that
     * are not supported by the importer should not be returned.
     *
     * @param repository_path Path to the repository to load.
     *
     * @return A list containing the path and contents of each loaded source
     * file, or an ImportError if the repository cannot be traversed.
     */
    virtual std::vector<std::pair<std::filesystem::path, std::string>>
    loadRepository(const std::filesystem::path &repository_path) const = 0;

    /**
     * @brief Provides the list of node definitions available to the importer.
     *
     * The returned descriptors allow the importer to associate source-code
     * node names with the corresponding core node identifiers.
     *
     * @return A span containing all node definitions provided by the
     * capability.
     */
    virtual std::span<const ImportNodeDescriptor> nodes() const noexcept = 0;

    /**
     * @brief Provides the list of type definitions available to the importer.
     *
     * The returned descriptors allow the importer to associate source-code
     * type names with the corresponding core data type identifiers.
     *
     * @return A span containing all type definitions provided by the
     * capability.
     */
    virtual std::span<const ImportTypeDescriptor> types() const noexcept = 0;
};

}  // namespace core::capa