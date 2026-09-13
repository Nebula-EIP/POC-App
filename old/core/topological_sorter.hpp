#pragma once

#include <cstdint>
#include <vector>

namespace core {

class Graph;
class NodeBase;

class TopologicalSorter {
   public:
    /**
     * @brief Sort a graph topologically using Kahn's algorithm.
     * @param graph The validated graph to sort
     * @return Vector of NodeBase pointers in topological order
     *         Each node appears after all its dependencies
     */
    static std::vector<NodeBase *> Sort(const Graph &graph);

   private:
    /**
     * @brief Calculate in-degree for a specific node.
     * @param node The node to analyze
     * @return Number of incoming edges
     */
    static uint32_t CalculateInDegree(NodeBase *node);

    /**
     * @brief Get all neighbors of a node (nodes it points to).
     * @param node The source node
     * @param graph The graph context
     * @return Vector of unique neighbor NodeBase pointers
     */
    static std::vector<NodeBase *> GetNeighbors(NodeBase *node,
                                                const Graph &graph);
};

}  // namespace core
