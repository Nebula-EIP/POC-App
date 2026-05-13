#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core {

// Forward declarations
class Graph;
class NodeBase;

/**
 * @class GraphValidator
 * @brief Validates a graph for correctness before code generation.
 *
 * GraphValidator performs three main checks:
 * 1. Cycle detection using colored DFS (WHITE/GRAY/BLACK)
 * 2. Pin type compatibility verification
 * 3. Required pin connectivity verification
 */
class GraphValidator {
   public:
    /**
     * @struct ValidationError
     * @brief Represents a single validation error found in the graph.
     */
    struct ValidationError {
        enum class ErrorType {
            kCycleDetected,             ///< Circular dependency found
            kIncompatiblePinTypes,      ///< Pin types don't match
            kMissingRequiredInputPin,   ///< Required input pin not connected
            kMissingRequiredOutputPin,  ///< Required output pin not connected
            kInvalidPinIndex,           ///< Pin index out of bounds
            kNullNodePointer,           ///< Node pointer is null
            kOrphanedNode               ///< Node has no connections (optional)
        };

        ErrorType type;
        std::string message;
        std::vector<uint32_t>
            involved_node_ids;  ///< Node IDs involved in error
        std::vector<std::pair<uint32_t, uint8_t>>
            involved_pins;  ///< (node_id, pin_index) pairs
    };

    /**
     * @struct ValidationResult
     * @brief Complete validation result for a graph.
     */
    struct ValidationResult {
        bool is_valid;  ///< true if no errors found
        std::vector<ValidationError> errors;

        size_t ErrorCount() const { return errors.size(); }
        bool HasCycleError() const;
        bool HasTypeError() const;
        bool HasPinConnectivityError() const;
    };

    GraphValidator() = default;
    ~GraphValidator() = default;

    /**
     * @brief Validates a graph for correctness.
     *
     * Performs all validation checks:
     * - Cycle detection (DFS colored)
     * - Pin type compatibility
     * - Required input pin connectivity
     *
     * @param graph The graph to validate
     * @return ValidationResult containing all found errors and validity status
     */
    ValidationResult Validate(const Graph &graph);

   private:
    // DFS color states for cycle detection
    enum class DFSColor {
        kWhite,  ///< Not yet visited
        kGray,   ///< Currently being processed (part of current DFS path)
        kBlack   ///< Fully processed (and ancestors)
    };

    /**
     * @brief Detects cycles using DFS with coloring.
     *
     * @param graph The graph to check
     * @return Vector of ValidationErrors for any cycles found
     */
    std::vector<ValidationError> DetectCycles(const Graph &graph);

    /**
     * @brief Recursively performs DFS for cycle detection.
     *
     * @param node_id Current node being processed
     * @param colors Color state of each node
     * @param parent_id Previous node in path (for error reporting)
     * @param graph Reference graph
     * @param errors Output: accumulates cycle errors
     * @return true if a cycle was found in this subtree
     */
    bool DFSVisit(uint32_t node_id,
                  std::unordered_map<uint32_t, DFSColor> &colors,
                  uint32_t parent_id, const Graph &graph,
                  std::vector<ValidationError> &errors);

    /**
     * @brief Validates pin type compatibility on all edges.
     *
     * For each connection in the graph, verifies that:
     * - Source pin type matches destination pin type, OR
     * - Types are implicitly convertible (int→float, etc.)
     *
     * @param graph The graph to check
     * @return Vector of ValidationErrors for any type mismatches
     */
    std::vector<ValidationError> CheckPinCompatibility(const Graph &graph);

    /**
     * @brief Validates that all required input pins are connected.
     *
     * For each node, checks that all input pins that require a connection
     * are actually connected to something.
     *
     * @param graph The graph to check
     * @return Vector of ValidationErrors for missing connections
     */
    std::vector<ValidationError> CheckPinConnectivity(const Graph &graph);

    /**
     * @brief Checks if two pin types are compatible.
     *
     * Compatible means:
     * - Exact match (int==int)
     * - Implicit conversion (int→float, etc.)
     *
     * @param source_type Type of source pin (as int from enum)
     * @param target_type Type of target pin (as int from enum)
     * @return true if compatible
     */
    static bool AreTypesCompatible(int source_type, int target_type);

    /**
     * @brief Helper to convert pin type to string for error messages.
     */
    static std::string PinTypeToString(int type);

    /**
     * @brief Helper to find a node by ID in the graph.
     */
    static NodeBase *FindNodeById(const Graph &graph, uint32_t id);
};

}  // namespace core
