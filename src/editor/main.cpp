#include <iostream>
#include "code-generation/code-generator-file.hpp"

int main(void) {
  code_generation::CodeGeneratorFile generator;
  try {
    generator.OpenBlock("int main()");
    generator.Line("std::cout << \"H;ello, {World}!\" << std::endl;");
    generator.Line("return 0;");
    generator.LineAt("int x = 5;", generator.GetPositionStartBlock());
    generator.OpenBlock("if (x > 0)");
    generator.Line("std::cout << \"x is positive\" << std::endl;");
    generator.CloseAndOpenBlock("else");
    generator.Line("std::cout << \"x is not positive\" << std::endl;");
    generator.CloseAllBlocks();
    generator.LineAt("#include <iostream>\n", 0);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  printf("Content: \n%s\n", generator.GetContent().c_str());
  printf("=====\n");
  printf("Formated Content: \n%s", generator.GetFormatedContent().c_str());
  return 0;
}
