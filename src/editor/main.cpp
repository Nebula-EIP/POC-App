#include <iostream>
#include "code-generation/code-generator-file.hpp"

int main(void) {
  code_generation::CodeGeneratorFile generator;
  generator.AddContent("int main() {");
  generator.AddContent("std::cout << \"H;ello, {World}!\" << std::endl;");
  generator.AddContent("return 0;");
  generator.AddContent("}");
  generator.AddContentAt("#include <iostream>\n", 0);

  printf("Content: \n%s\n", generator.GetContent().c_str());
  printf("=====\n");
  printf("Formated Content: \n%s", generator.GetFormatedContent().c_str());
  return 0;
}
