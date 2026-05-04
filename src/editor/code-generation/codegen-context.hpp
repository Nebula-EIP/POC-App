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
};

}  // namespace editor::code_generation
