
#include "lexer.hpp"
#include "parser.hpp"
#include "codegen.hpp"
#include "optimizer.hpp"
#include "vm.hpp"

#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    std::string mode = "--run";
    std::string filePath;

    // Parse command-line arguments.
    if (argc == 2) {
        filePath = argv[1];
    } else if (argc == 3) {
        mode = argv[1];
        filePath = argv[2];
    } else {
        std::cerr
            << "Usage: tinyc [--tokens|--ast|--bytecode|"
            << "--optimized-bytecode|--optimized|--stats] "
            << "<source-file>\n";
        return 1;
    }

    // Validate the requested mode.
    if (mode != "--run" &&
        mode != "--tokens" &&
        mode != "--ast" &&
        mode != "--bytecode" &&
        mode != "--optimized-bytecode" &&
        mode != "--optimized" &&
        mode != "--stats") {
        std::cerr << "Unknown mode: " << mode << '\n';
        return 1;
    }

    // Read the source file.
    std::ifstream input(filePath);

    if (!input) {
        std::cerr << "Error: cannot open file: "
                  << filePath << '\n';
        return 1;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();

    // Stage 1: Lexical analysis.
    Lexer lexer(buffer.str());
    std::vector<Token> tokens = lexer.tokenize();

    if (mode == "--tokens") {
        bool hasError = false;

        for (const Token& token : tokens) {
            std::cout << tokenTypeName(token.type)
                      << "  \"" << token.lexeme << "\""
                      << "  line " << token.line
                      << ", column " << token.column
                      << '\n';

            if (token.type == TokenType::Invalid) {
                hasError = true;
            }
        }

        return hasError ? 1 : 0;
    }

    // Reject invalid characters before parsing.
    for (const Token& token : tokens) {
        if (token.type == TokenType::Invalid) {
            std::cerr << "Lexical error at "
                      << token.line << ':'
                      << token.column
                      << ": unexpected '"
                      << token.lexeme << "'\n";
            return 1;
        }
    }

    // Stage 2: Parsing and AST construction.
    std::unique_ptr<Program> program;

    try {
        Parser parser(tokens);
        program = std::make_unique<Program>(parser.parse());
    } catch (const ParseError& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    if (mode == "--ast") {
        program->dump(std::cout);
        return 0;
    }

    // Stage 3: Generate bytecode.
    CodeGenerator generator;
    std::vector<Instruction> bytecode;

    try {
        bytecode = generator.generate(*program);
    } catch (const std::exception& error) {
        std::cerr << "Compilation error: "
                  << error.what() << '\n';
        return 1;
    }

    // Report static instruction-count reduction.
    if (mode == "--stats") {
        const std::vector<Instruction> optimized =
            optimizeBytecode(bytecode);

        const std::size_t before = bytecode.size();
        const std::size_t after = optimized.size();

        const std::size_t removed =
            before >= after ? before - after : 0;

        const double reduction = before == 0
            ? 0.0
            : 100.0 * static_cast<double>(removed)
                      / static_cast<double>(before);

        std::cout << "Original instructions: "
                  << before << '\n';

        std::cout << "Optimized instructions: "
                  << after << '\n';

        std::cout << "Instructions removed: "
                  << removed << '\n';

        std::cout << std::fixed << std::setprecision(2)
                  << "Instruction reduction: "
                  << reduction << "%\n";

        return 0;
    }

    // Stage 4: Optional optimization.
    const bool optimizedMode =
        mode == "--optimized" ||
        mode == "--optimized-bytecode";

    if (optimizedMode) {
        bytecode = optimizeBytecode(bytecode);
    }

    // Inspect bytecode without executing it.
    if (mode == "--bytecode" ||
        mode == "--optimized-bytecode") {
        dumpBytecode(bytecode, std::cout);
        return 0;
    }

    // Stage 5: Execute bytecode.
    try {
        VirtualMachine vm(std::cout);
        vm.run(bytecode);
    } catch (const std::exception& error) {
        std::cerr << "Runtime error: "
                  << error.what() << '\n';
        return 1;
    }

    return 0;
}
