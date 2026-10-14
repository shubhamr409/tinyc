
#pragma once

#include "ast.hpp"
#include "token.hpp"

#include <initializer_list>
#include <stdexcept>
#include <vector>

class ParseError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    Program parse();

private:
    const std::vector<Token>& tokens_;
    std::size_t current_ = 0;

    const Token& peek() const;
    const Token& previous() const;
    bool isAtEnd() const;

    Token advance();
    bool check(TokenType type) const;

    bool match(std::initializer_list<TokenType> types);

    Token consume(TokenType type, const char* message);

    ParseError errorAt(const Token& token,
                       const std::string& message) const;

    StmtPtr declaration();
    StmtPtr letDeclaration();
    StmtPtr printStatement();
    StmtPtr assignmentStatement();
    StmtPtr whileStatement();
    ExprPtr expression();
    ExprPtr comparison();
    ExprPtr addition();
    ExprPtr multiplication();
    ExprPtr unary();
    ExprPtr primary();
};
