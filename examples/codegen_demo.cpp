#include <iostream>
#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "src/core/graph.hpp"
#include "src/core/nodes/literal_node.hpp"
#include "src/core/nodes/operator_node.hpp"
#include "src/core/nodes/print_node.hpp"
#include "src/editor/code-generation/codegen-context.hpp"

namespace {

bool WriteTextFile(const std::filesystem::path &path,
                   const std::string &content) {
    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }
    out << content;
    return out.good();
}

std::string RunCommandCaptureOutput(const std::string &command,
                                    int &exit_code) {
    std::array<char, 256> buffer{};
    std::string output;

    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        exit_code = -1;
        return "";
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) {
        output += buffer.data();
    }

    exit_code = pclose(pipe);
    return output;
}

}  // namespace

int main() {
    editor::code_generation::CodegenContext ctx;

    // Example 1: arithmetic graph (3 + 4) * 2
    core::Graph arithmetic_graph;

    auto *lit1 = arithmetic_graph.AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *lit2 = arithmetic_graph.AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *lit3 = arithmetic_graph.AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral, {0, 0});

    auto *add_op = arithmetic_graph.AddNode<core::OperatorNode>(
        core::NodeBase::NodeKind::kOperator, {0, 0});
    auto *mult_op = arithmetic_graph.AddNode<core::OperatorNode>(
        core::NodeBase::NodeKind::kOperator, {0, 0});

    lit1->set_data(3);
    lit2->set_data(4);
    lit3->set_data(2);

    add_op->SetOperatorType(core::OperatorNode::OperatorType::kAddition);
    mult_op->SetOperatorType(core::OperatorNode::OperatorType::kMultiplication);

    arithmetic_graph.Link(lit1, 0, add_op, 0);
    arithmetic_graph.Link(lit2, 0, add_op, 1);
    arithmetic_graph.Link(add_op, 0, mult_op, 0);
    arithmetic_graph.Link(lit3, 0, mult_op, 1);

    std::cout << "===== EXAMPLE 1: ARITHMETIC (STANDARD) =====\n";
    auto arithmetic_standard = ctx.Generate(arithmetic_graph);
    std::cout << arithmetic_standard.GetFormatedContent() << "\n";

    std::cout << "\n===== EXAMPLE 1: ARITHMETIC (WITH OUTPUTS) =====\n";
    auto arithmetic_with_outputs = ctx.GenerateWithOutputs(arithmetic_graph, true);
    std::cout << arithmetic_with_outputs.GetFormatedContent() << "\n";

    // Export generated arithmetic file
    const std::filesystem::path kGeneratedDir = "examples/output";
    std::filesystem::create_directories(kGeneratedDir);
    const std::filesystem::path kArithmeticCpp = kGeneratedDir / "arithmetic_graph.cpp";
    const std::filesystem::path kArithmeticBin = kGeneratedDir / "arithmetic_graph";

    if (!WriteTextFile(kArithmeticCpp, arithmetic_with_outputs.GetFormatedContent())) {
        std::cerr << "[ERROR] Failed to write: " << kArithmeticCpp << '\n';
        return 1;
    }

    std::cout << "[INFO] Exported file: " << kArithmeticCpp << '\n';

    // Compile and execute exported arithmetic file
    int compile_exit = 0;
    const std::string kCompileArithmetic =
        "g++ -std=c++23 \"" + kArithmeticCpp.string() + "\" -o \"" +
        kArithmeticBin.string() + "\" 2>&1";
    const std::string kCompileArithmeticOutput =
        RunCommandCaptureOutput(kCompileArithmetic, compile_exit);

    if (compile_exit != 0) {
        std::cerr << "[ERROR] Failed to compile exported arithmetic file\n";
        std::cerr << kCompileArithmeticOutput << '\n';
        return 2;
    }

    int run_exit = 0;
    const std::string kRunArithmetic = "\"" + kArithmeticBin.string() + "\" 2>&1";
    const std::string kRunArithmeticOutput =
        RunCommandCaptureOutput(kRunArithmetic, run_exit);

    if (run_exit != 0) {
        std::cerr << "[ERROR] Exported arithmetic executable failed\n";
        std::cerr << kRunArithmeticOutput << '\n';
        return 3;
    }

    std::cout << "[INFO] Arithmetic executable output:\n" << kRunArithmeticOutput << '\n';

    // Expected result for (3 + 4) * 2 is 14.
    if (kRunArithmeticOutput.find("14") == std::string::npos) {
        std::cerr << "[ERROR] Unexpected arithmetic output: expected value 14\n";
        return 4;
    }

    std::cout << "[OK] Arithmetic export/compile/run validation passed.\n";

    // Example 2: print node from a string literal
    core::Graph print_graph;
    auto *message = print_graph.AddNode<core::LiteralNode>(
        core::NodeBase::NodeKind::kLiteral, {0, 0});
    auto *printer = print_graph.AddNode<core::PrintNode>(
        core::NodeBase::NodeKind::kPrint, {0, 0});

    message->SetType(core::NodeBase::PinDataType::kString);
    message->set_data(std::string("Hello from PrintNode!"));

    print_graph.Link(message, 0, printer, 0);

    std::cout << "\n===== EXAMPLE 2: PRINT NODE =====\n";
    auto print_code = ctx.Generate(print_graph);
    std::cout << print_code.GetFormatedContent() << "\n";

    // Export generated print file
    const std::filesystem::path kPrintCpp = kGeneratedDir / "print_graph.cpp";
    const std::filesystem::path kPrintBin = kGeneratedDir / "print_graph";

    if (!WriteTextFile(kPrintCpp, print_code.GetFormatedContent())) {
        std::cerr << "[ERROR] Failed to write: " << kPrintCpp << '\n';
        return 5;
    }

    std::cout << "[INFO] Exported file: " << kPrintCpp << '\n';

    int compile_print_exit = 0;
    const std::string kCompilePrint =
        "g++ -std=c++23 \"" + kPrintCpp.string() + "\" -o \"" +
        kPrintBin.string() + "\" 2>&1";
    const std::string kCompilePrintOutput =
        RunCommandCaptureOutput(kCompilePrint, compile_print_exit);

    if (compile_print_exit != 0) {
        std::cerr << "[ERROR] Failed to compile exported print file\n";
        std::cerr << kCompilePrintOutput << '\n';
        return 6;
    }

    int run_print_exit = 0;
    const std::string kRunPrint = "\"" + kPrintBin.string() + "\" 2>&1";
    const std::string kRunPrintOutput =
        RunCommandCaptureOutput(kRunPrint, run_print_exit);

    if (run_print_exit != 0) {
        std::cerr << "[ERROR] Exported print executable failed\n";
        std::cerr << kRunPrintOutput << '\n';
        return 7;
    }

    std::cout << "[INFO] Print executable output:\n" << kRunPrintOutput << '\n';

    if (kRunPrintOutput.find("Hello from PrintNode!") == std::string::npos) {
        std::cerr << "[ERROR] Unexpected print output\n";
        return 8;
    }

    std::cout << "[OK] Print export/compile/run validation passed.\n";

    return 0;
}
