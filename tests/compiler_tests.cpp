
#include "lexer.hpp"
#include "parser.hpp"
#include "codegen.hpp"
#include "optimizer.hpp"
#include "vm.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

int passed = 0;
int failed = 0;

void check(bool condition, const std::string& name) {
    if (condition) {
        ++passed;
        std::cout << "[PASS] " << name << '\n';
    } else {
        ++failed;
        std::cout << "[FAIL] " << name << '\n';
    }
}

std::vector<Instruction> compileSource(
    const std::string& source) {

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    for (const Token& token : tokens) {
        if (token.type == TokenType::Invalid) {
            throw std::runtime_error("Invalid token");
        }
    }

    Parser parser(tokens);
    Program program = parser.parse();

    CodeGenerator generator;
    return generator.generate(program);
}

struct RunResult {
    bool success;
    std::string output;
    std::string error;
};

RunResult executeCode(
    const std::vector<Instruction>& code) {

    std::ostringstream output;

    try {
        VirtualMachine vm(output);
        vm.run(code);

        return {true, output.str(), ""};
    } catch (const std::exception& error) {
        return {false, output.str(), error.what()};
    }
}

std::string bytecodeText(
    const std::vector<Instruction>& code) {

    std::ostringstream output;
    dumpBytecode(code, output);
    return output.str();
}

} // namespace

int main() {
    try {
        // 1. Operator precedence.
        {
            auto code = compileSource("print 2 + 3 * 4;");
            auto result = executeCode(code);

            check(
                result.success && result.output == "14\n",
                "Arithmetic precedence"
            );
        }

        // 2. Variables and assignment.
        {
            auto code = compileSource(
                "let x = 10; x = x + 2; print x;"
            );

            auto result = executeCode(code);

            check(
                result.success && result.output == "12\n",
                "Variables and assignment"
            );
        }

        // 3. Optimized and unoptimized execution agree.
        {
            auto code = compileSource(
                "let answer = 2 + 3 * (4 - 1);"
                "answer = answer - 2;"
                "print answer; print -answer;"
            );

            auto optimized = optimizeBytecode(code);

            auto originalResult = executeCode(code);
            auto optimizedResult = executeCode(optimized);

            check(
                originalResult.success &&
                optimizedResult.success &&
                originalResult.output == "9\n-9\n" &&
                optimizedResult.output == originalResult.output,
                "Optimization preserves program output"
            );
        }

        // 4. Instruction-count reduction.
        {
            auto code = compileSource("print 2 + 3 * 4;");
            auto optimized = optimizeBytecode(code);

            auto result = executeCode(optimized);

            check(
                code.size() == 7 &&
                optimized.size() == 3 &&
                result.success &&
                result.output == "14\n",
                "Constant folding reduces 7 instructions to 3"
            );
        }

        // 5. Integer division follows VM semantics.
        {
            auto code = compileSource("print 7 / 2;");
            auto optimized = optimizeBytecode(code);

            auto result = executeCode(optimized);

            check(
                result.success &&
                result.output == "3\n" &&
                optimized.size() < code.size(),
                "Constant integer division"
            );
        }

        // 6. Division by zero remains a runtime error.
        {
            auto code = compileSource("print 10 / 0;");
            auto optimized = optimizeBytecode(code);

            auto originalResult = executeCode(code);
            auto optimizedResult = executeCode(optimized);

            check(
                !originalResult.success &&
                !optimizedResult.success &&
                originalResult.error.find("Division by zero") !=
                    std::string::npos &&
                optimizedResult.error == originalResult.error,
                "Division-by-zero behavior is preserved"
            );
        }

        // 7. Signed overflow remains a runtime error.
        {
            auto code = compileSource(
                "print 9223372036854775807 + 1;"
            );

            auto optimized = optimizeBytecode(code);

            auto originalResult = executeCode(code);
            auto optimizedResult = executeCode(optimized);

            check(
                !originalResult.success &&
                !optimizedResult.success &&
                originalResult.error.find("Integer overflow") !=
                    std::string::npos &&
                optimizedResult.error == originalResult.error,
                "Integer-overflow behavior is preserved"
            );
        }

        // 8. Undefined variables are rejected.
        {
            bool rejected = false;

            try {
                compileSource("print missing;");
            } catch (const std::exception& error) {
                rejected =
                    std::string(error.what()).find("undeclared") !=
                    std::string::npos;
            }

            check(rejected, "Undeclared-variable validation");
        }

        // 9. Optimization is idempotent for our current pass.
        {
            auto code = compileSource(
                "print 2 + 3 * 4;"
                "print 20 / 2;"
            );

            auto once = optimizeBytecode(code);
            auto twice = optimizeBytecode(once);

            check(
                bytecodeText(once) == bytecodeText(twice),
                "Repeated optimization produces identical bytecode"
            );
        }

        
        // 10. Basic while-loop execution.
        {
            auto code = compileSource(
                "let x = 5;"
                "while (x > 0) {"
                " print x;"
                " x = x - 1;"
                "}"
            );

            auto result = executeCode(code);

            check(
                result.success &&
                result.output == "5\n4\n3\n2\n1\n",
                "While-loop execution"
            );
        }

        // 11. Optimization must preserve jump targets.
        {
            auto code = compileSource(
                "let x = 3;"
                "while (x > 0) {"
                " print 2 + 3;"
                " x = x - 1;"
                "}"
            );

            auto optimized = optimizeBytecode(code);

            auto originalResult = executeCode(code);
            auto optimizedResult = executeCode(optimized);

            check(
                originalResult.success &&
                optimizedResult.success &&
                originalResult.output == "5\n5\n5\n" &&
                optimizedResult.output == originalResult.output &&
                optimized.size() < code.size(),
                "Optimized loops preserve jump targets"
            );
        }

        // 12. Reject declarations inside loops.
        {
            bool rejected = false;

            try {
                compileSource(
                    "let x = 2;"
                    "while (x > 0) {"
                    " let y = 1;"
                    " x = x - 1;"
                    "}"
                );
            } catch (const std::exception& error) {
                rejected =
                    std::string(error.what()).find("inside loops") !=
                    std::string::npos;
            }

            check(
                rejected,
                "Declarations inside loops are rejected"
            );
        }

        // 13. All comparison operators work on runtime values.
        {
            auto code = compileSource(
                "let x = 7;"
                "print x < 8;"
                "print x <= 7;"
                "print x >= 8;"
                "print x >= 7;"
                "print x == 7;"
                "print x != 7;"
            );

            auto optimized = optimizeBytecode(code);
            auto originalResult = executeCode(code);
            auto optimizedResult = executeCode(optimized);
            const std::string expected = "1\n1\n0\n1\n1\n0\n";

            check(
                originalResult.success &&
                optimizedResult.success &&
                originalResult.output == expected &&
                optimizedResult.output == expected,
                "All comparison operators work in both execution modes"
            );
        }

        // 14. Comparison precedence and constant folding.
        {
            auto code = compileSource(
                "print 1 + 2 < 4 == 1;"
                "print 2 * 3 >= 6;"
                "print 5 != 5;"
                "print 3 <= 2;"
            );

            auto optimized = optimizeBytecode(code);
            auto originalResult = executeCode(code);
            auto optimizedResult = executeCode(optimized);
            const std::string expected = "1\n1\n0\n0\n";

            check(
                originalResult.success &&
                optimizedResult.success &&
                originalResult.output == expected &&
                optimizedResult.output == expected &&
                optimized.size() < code.size(),
                "Comparison precedence and constant folding"
            );
        }

    } catch (const std::exception& error) {
        ++failed;
        std::cerr << "[FAIL] Unexpected test exception: "
                  << error.what() << '\n';
    }

    std::cout << "\nResults: " << passed
              << " passed, " << failed << " failed\n";

    return failed == 0 ? 0 : 1;
}

