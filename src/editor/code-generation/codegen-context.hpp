#pragma once

#include <string>

#include "code-generation/code-generator-file.hpp"

namespace core {
class Graph;
class NodeBase;
class GraphValidator;
class TopologicalSorter;
}  // namespace core

namespace editor::code_generation {

class CodegenContext {
   public:
    CodegenContext() = default;

    /**
     * @brief Generates C++ code from the given graph.
     * @param graph The input graph to generate code from
     * @param fold_constants If true, performs constant folding optimization
     * @return A CodeGeneratorFile containing the generated C++ code
     */
    ::code_generation::CodeGeneratorFile Generate(const core::Graph &graph,
                                                  bool fold_constants = true);

    /**
     * @brief Generates C++ code from the given graph, with options to print all
     * @param graph The input graph to generate code from
     * @param print_all_results If true, adds print statements for all node outputs
     * @param fold_constants If true, performs constant folding optimization
     * @return A CodeGeneratorFile containing the generated C++ code
     */
    ::code_generation::CodeGeneratorFile GenerateWithOutputs(
        const core::Graph &graph, bool print_all_results = false,
        bool fold_constants = true);
};

}  // namespace editor::code_generation
