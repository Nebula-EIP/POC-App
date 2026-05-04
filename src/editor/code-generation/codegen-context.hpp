#pragma once

#include "code-generation/code-generator-file.hpp"
#include <string>

namespace core {
class Graph;
class NodeBase;
}

namespace editor::code_generation {

class CodegenContext {
  public:
    CodegenContext() = default;

    // Generate a code file from the provided graph. Returns a
    // CodeGeneratorFile containing the generated C++ code.
    ::code_generation::CodeGeneratorFile Generate(const core::Graph &graph);
    
    // Generate with enhanced main() that prints labeled outputs
    // Named outputs: if true, prints all computed results with labels
    ::code_generation::CodeGeneratorFile GenerateWithOutputs(
        const core::Graph &graph,
        bool print_all_results = false);
};

}  // namespace editor::code_generation
