
#include "codegen.hpp"

#include <stdexcept>
#include <string>

void CodeGenerator::emit(
    OpCode op,
    std::int64_t operand,
    const std::string& name) {

    code_.push_back({op, operand, name});
}

std::vector<Instruction> CodeGenerator::generate(
    const Program& program) {

    code_.clear();
    declared_.clear();
    loopDepth_ = 0;

    for (const auto& statement : program.statements) {
        compileStatement(*statement);
    }

    emit(OpCode::Halt);
    return code_;
}

void CodeGenerator::compileStatement(const Stmt& statement) {
    if (const auto* node = dynamic_cast<const LetStmt*>(&statement)) {
        if (loopDepth_ > 0) {
            throw std::runtime_error(
                "Variable declarations inside loops are not supported; "
                "declare variables before the loop"
            );
        }
        if (declared_.count(node->name)) {
            throw std::runtime_error(
                "Variable '" + node->name +
                "' is already declared"
            );
        }

        // Evaluate the initializer first.
        compileExpression(*node->initializer);

        emit(OpCode::Declare, 0, node->name);
        declared_.insert(node->name);
        return;
    }

    if (const auto* node = dynamic_cast<const AssignStmt*>(&statement)) {
        if (!declared_.count(node->name)) {
            throw std::runtime_error(
                "Cannot assign to undeclared variable '" +
                node->name + "'"
            );
        }

        compileExpression(*node->value);
        emit(OpCode::Store, 0, node->name);
        return;
    }

    if (const auto* node = dynamic_cast<const PrintStmt*>(&statement)) {
        compileExpression(*node->expression);
        emit(OpCode::Print);
        return;
    }
    
    if (const auto* node =
            dynamic_cast<const WhileStmt*>(&statement)) {

        // Start of the loop condition.
        const std::size_t loopStart = code_.size();

        compileExpression(*node->condition);

        // Exit when the condition evaluates to zero.
        const std::size_t exitJump = code_.size();
        emit(OpCode::JumpIfFalse);

        ++loopDepth_;

        try {
            for (const auto& child : node->body) {
                compileStatement(*child);
            }
        } catch (...) {
            --loopDepth_;
            throw;
        }

        --loopDepth_;

        // Return to the condition.
        emit(
            OpCode::Jump,
            static_cast<std::int64_t>(loopStart)
        );

        // Patch the exit target.
        code_[exitJump].operand =
            static_cast<std::int64_t>(code_.size());

        return;
    }

    throw std::runtime_error("Unknown statement type");
}

void CodeGenerator::compileExpression(const Expr& expression) {
    if (const auto* node = dynamic_cast<const IntegerExpr*>(&expression)) {
        emit(OpCode::PushConst, node->value);
        return;
    }

    if (const auto* node = dynamic_cast<const VariableExpr*>(&expression)) {
        if (!declared_.count(node->name)) {
            throw std::runtime_error(
                "Use of undeclared variable '" + node->name + "'"
            );
        }

        emit(OpCode::Load, 0, node->name);
        return;
    }

    if (const auto* node = dynamic_cast<const UnaryExpr*>(&expression)) {
        compileExpression(*node->operand);

        if (node->op == TokenType::Minus) {
            emit(OpCode::Neg);
            return;
        }

        throw std::runtime_error("Unsupported unary operator");
    }

    if (const auto* node = dynamic_cast<const BinaryExpr*>(&expression)) {
        compileExpression(*node->left);
        compileExpression(*node->right);

        switch (node->op) {
            case TokenType::Plus:
                emit(OpCode::Add);
                break;
            case TokenType::Less:
                emit(OpCode::Less);
                break;

            case TokenType::LessEqual:
                emit(OpCode::LessEqual);
                break;

            case TokenType::Greater:
                emit(OpCode::Greater);
                break;

            case TokenType::GreaterEqual:
                emit(OpCode::GreaterEqual);
                break;

            case TokenType::EqualEqual:
                emit(OpCode::Equal);
                break;

            case TokenType::BangEqual:
                emit(OpCode::NotEqual);
                break;

            case TokenType::Minus:
                emit(OpCode::Sub);
                break;

            case TokenType::Star:
                emit(OpCode::Mul);
                break;

            case TokenType::Slash:
                emit(OpCode::Div);
                break;

            default:
                throw std::runtime_error("Unsupported binary operator");
        }

        return;
    }

    throw std::runtime_error("Unknown expression type");
}
