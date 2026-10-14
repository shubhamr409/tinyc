
#pragma once

#include "ast.hpp"
#include "bytecode.hpp"

#include <unordered_set>
#include <vector>

class CodeGenerator {
public:
    std::vector<Instruction> generate(const Program& program);

private:
    std::vector<Instruction> code_;
    std::unordered_set<std::string> declared_;
    int loopDepth_ = 0;

    void emit(OpCode op, std::int64_t operand = 0,
              const std::string& name = "");

    void compileStatement(const Stmt& statement);
    void compileExpression(const Expr& expression);
};
