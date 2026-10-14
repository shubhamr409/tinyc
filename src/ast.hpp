
#pragma once

#include "token.hpp"
#include <utility>
#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

inline void printIndent(std::ostream& out, int level) {
    for (int i = 0; i < level; ++i) {
        out << ' ';
    }
}

struct Expr {
    virtual ~Expr() = default;

    virtual void dump(std::ostream& out, int indent) const = 0;
};

using ExprPtr = std::unique_ptr<Expr>;

struct IntegerExpr final : Expr {
    std::int64_t value;

    explicit IntegerExpr(std::int64_t value)
        : value(value) {}

    void dump(std::ostream& out, int indent) const override {
        printIndent(out, indent);
        out << "Integer " << value << '\n';
    }
};

struct VariableExpr final : Expr {
    std::string name;

    explicit VariableExpr(std::string name)
        : name(std::move(name)) {}

    void dump(std::ostream& out, int indent) const override {
        printIndent(out, indent);
        out << "Variable " << name << '\n';
    }
};

struct UnaryExpr final : Expr {
    TokenType op;
    ExprPtr operand;

    UnaryExpr(TokenType op, ExprPtr operand)
        : op(op), operand(std::move(operand)) {}

    void dump(std::ostream& out, int indent) const override {
        printIndent(out, indent);
        out << "Unary " << tokenTypeName(op) << '\n';
        operand->dump(out, indent + 2);
    }
};

struct BinaryExpr final : Expr {
    ExprPtr left;
    TokenType op;
    ExprPtr right;

    BinaryExpr(ExprPtr left, TokenType op, ExprPtr right)
        : left(std::move(left)),
          op(op),
          right(std::move(right)) {}

    void dump(std::ostream& out, int indent) const override {
        printIndent(out, indent);
        out << "Binary " << tokenTypeName(op) << '\n';

        left->dump(out, indent + 2);
        right->dump(out, indent + 2);
    }
};

struct Stmt {
    virtual ~Stmt() = default;

    virtual void dump(std::ostream& out, int indent) const = 0;
};

using StmtPtr = std::unique_ptr<Stmt>;

struct LetStmt final : Stmt {
    std::string name;
    ExprPtr initializer;

    LetStmt(std::string name, ExprPtr initializer)
        : name(std::move(name)),
          initializer(std::move(initializer)) {}

    void dump(std::ostream& out, int indent) const override {
        printIndent(out, indent);
        out << "Let " << name << '\n';

        initializer->dump(out, indent + 2);
    }
};

struct AssignStmt final : Stmt {
    std::string name;
    ExprPtr value;

    AssignStmt(std::string name, ExprPtr value)
        : name(std::move(name)),
          value(std::move(value)) {}

    void dump(std::ostream& out, int indent) const override {
        printIndent(out, indent);
        out << "Assign " << name << '\n';

        value->dump(out, indent + 2);
    }
};

struct PrintStmt final : Stmt {
    ExprPtr expression;

    explicit PrintStmt(ExprPtr expression)
        : expression(std::move(expression)) {}

    void dump(std::ostream& out, int indent) const override {
        printIndent(out, indent);
        out << "Print\n";

        expression->dump(out, indent + 2);
    }
};

struct WhileStmt final : Stmt {
    ExprPtr condition;
    std::vector<StmtPtr> body;

    WhileStmt(ExprPtr condition, std::vector<StmtPtr> body)
        : condition(std::move(condition)),
          body(std::move(body)) {}

    void dump(std::ostream& out, int indent) const override {
        printIndent(out, indent);
        out << "While\n";

        printIndent(out, indent + 2);
        out << "Condition\n";
        condition->dump(out, indent + 4);

        printIndent(out, indent + 2);
        out << "Body\n";

        for (const auto& statement : body) {
            statement->dump(out, indent + 4);
        }
    }
};


struct Program {
    std::vector<StmtPtr> statements;

    void dump(std::ostream& out) const {
        out << "Program\n";

        for (const auto& statement : statements) {
            statement->dump(out, 2);
        }
    }
};
