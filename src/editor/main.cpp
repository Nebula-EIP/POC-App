#include <iostream>

#include "code-generation/code-generator-file.hpp"
#include "file-writing/file-writer.hpp"

int main(void) {
    code_generation::CodeGeneratorFile generator;
    file_writing::FileWriter writer;

    try {
        generator.OpenBlock("int main()");
        generator.Line("std::cout << \"H;ello, {World}!\" << std::endl;");
        generator.LineAt("int x = 5;", generator.GetPositionStartBlock());
        generator.OpenBlock("if (x > 0)");
        generator.Line("std::cout << \"x is positive\" << std::endl;");
        generator.Line("return 1;");
        generator.CloseAndOpenBlock("else");
        generator.Line("std::cout << \"x is not positive\" << std::endl;");
        generator.Line("return 0;");
        generator.CloseAllBlocks();
        generator.LineAt("#include <iostream>\n", 0);
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    printf("Content: \n%s\n", generator.GetContent().c_str());
    printf("=====\n");
    printf("Formated Content: \n%s", generator.GetFormatedContent().c_str());
    writer.WriteToFile("code-generated/main.cpp",
                       generator.GetFormatedContent());
    return 0;
}
