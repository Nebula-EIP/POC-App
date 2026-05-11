#pragma once

#include <vector>
#include <cstdint>

namespace core {

// Forward declarations
class Graph;
class NodeBase;

/**
 * @class TopologicalSorter
 * @brief Performs topological sorting of a graph using Kahn's algorithm.
 *
 * Kahn's algorithm:
 * 1. Calculate in-degree for each node (count of incoming edges)
 * 2. Initialize queue with all nodes having in-degree 0
 * 3. While queue not empty:
 *    - Dequeue node, add to sorted list
 *    - For each neighbor, decrease in-degree
 *    - If neighbor's in-degree becomes 0, enqueue it
 * 4. If sorted list size != graph size, graph has cycle (shouldn't happen after validation)
 *
 * Time complexity: O(V + E) where V = vertices, E = edges
 * Space complexity: O(V)
 */
class TopologicalSorter {
   public:
    /**
     * @brief Sort a graph topologically using Kahn's algorithm.
     *
     * @param graph The validated graph to sort
     * @return Vector of NodeBase pointers in topological order
     *         Each node appears after all its dependencies
     *
     * @note Graph must be validated before sorting (no cycles)
     * @note Empty graph returns empty vector
     */
    static std::vector<NodeBase*> Sort(const Graph& graph);

   private:
    /**
     * @brief Calculate in-degree for a specific node.
     *
     * In-degree = number of incoming connections (parents)
     *
     * @param node The node to analyze
     * @return Number of incoming edges
     */
    static uint32_t CalculateInDegree(NodeBase* node);

    /**
     * @brief Get all neighbors of a node (nodes it points to).
     *
     * @param node The source node
     * @param graph The graph context
     * @return Vector of unique neighbor NodeBase pointers
     */
    static std::vector<NodeBase*> GetNeighbors(NodeBase* node, const Graph& graph);
};

}  // namespace core
