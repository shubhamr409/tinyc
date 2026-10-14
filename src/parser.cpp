
#include "parser.hpp"

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens) {}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EndOfFile;
}

Token Parser::advance() {
    if (!isAtEnd()) {
        ++current_;
    }

    return tokens_[current_ - 1];
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }

    return false;
}

Token Parser::consume(TokenType type, const char* message) {
    if (check(type)) {
        return advance();
    }

    throw errorAt(peek(), message);
}

ParseError Parser::errorAt(
    const Token& token,
    const std::string& message) const {

    return ParseError(
        "Parse error at " +
        std::to_string(token.line) + ":" +
        std::to_string(token.column) + ": " +
        message
    );
}

Program Parser::parse() {
    Program program;

    while (!isAtEnd()) {
        program.statements.push_back(declaration());
    }

    return program;
}

StmtPtr Parser::declaration() {
    if (match({TokenType::Let})) {
        return letDeclaration();
    }

    if (match({TokenType::Print})) {
        return printStatement();
    }

    if (match({TokenType::While})) {
        return whileStatement();
    }

    if (check(TokenType::Identifier) &&
        current_ + 1 < tokens_.size() &&
        tokens_[current_ + 1].type == TokenType::Equal) {
        return assignmentStatement();
    }

    if (check(TokenType::Invalid)) {
        throw errorAt(peek(), "Invalid character");
    }

    throw errorAt(
        peek(),
        "Expected a variable declaration, assignment, "
        "print statement, or while loop"
    );
}

StmtPtr Parser::letDeclaration() {
    Token name = consume(
        TokenType::Identifier,
        "Expected a variable name after 'let'"
    );

    consume(
        TokenType::Equal,
        "Expected '=' after the variable name"
    );

    ExprPtr initializer = expression();

    consume(
        TokenType::Semicolon,
        "Expected ';' after variable declaration"
    );

    return std::make_unique<LetStmt>(
        name.lexeme,
        std::move(initializer)
    );
}

StmtPtr Parser::printStatement() {
    ExprPtr value = expression();

    consume(
        TokenType::Semicolon,
        "Expected ';' after print expression"
    );

    return std::make_unique<PrintStmt>(
        std::move(value)
    );
}

StmtPtr Parser::whileStatement() {
    consume(
        TokenType::LeftParen,
        "Expected '(' after 'while'"
    );

    ExprPtr condition = expression();

    consume(
        TokenType::RightParen,
        "Expected ')' after while condition"
    );

    consume(
        TokenType::LeftBrace,
        "Expected '{' before while body"
    );

    std::vector<StmtPtr> body;

    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        body.push_back(declaration());
    }

    consume(
        TokenType::RightBrace,
        "Expected '}' after while body"
    );

    return std::make_unique<WhileStmt>(
        std::move(condition),
        std::move(body)
    );
}

StmtPtr Parser::assignmentStatement() {
    Token name = consume(
        TokenType::Identifier,
        "Expected variable name"
    );

    consume(
        TokenType::Equal,
        "Expected '=' after variable name"
    );

    ExprPtr value = expression();

    consume(
        TokenType::Semicolon,
        "Expected ';' after assignment"
    );

    return std::make_unique<AssignStmt>(
        name.lexeme,
        std::move(value)
    );
}

// expression -> equality
// equality -> comparison (('==' | '!=') comparison)*
// comparison -> addition (('<' | '<=' | '>' | '>=') addition)*

ExprPtr Parser::expression() {
    ExprPtr expr = comparison();

    while (match({TokenType::EqualEqual, TokenType::BangEqual})) {
        TokenType op = previous().type;
        ExprPtr right = comparison();

        expr = std::make_unique<BinaryExpr>(
            std::move(expr),
            op,
            std::move(right)
        );
    }

    return expr;
}

ExprPtr Parser::comparison() {
    ExprPtr expr = addition();

    while (match({
        TokenType::Less,
        TokenType::LessEqual,
        TokenType::Greater,
        TokenType::GreaterEqual
    })) {
        TokenType op = previous().type;
        ExprPtr right = addition();

        expr = std::make_unique<BinaryExpr>(
            std::move(expr),
            op,
            std::move(right)
        );
    }

    return expr;
}

// addition -> multiplication (('+' | '-') multiplication)*

ExprPtr Parser::addition() {
    ExprPtr expr = multiplication();

    while (match({TokenType::Plus, TokenType::Minus})) {
        TokenType op = previous().type;
        ExprPtr right = multiplication();

        expr = std::make_unique<BinaryExpr>(
            std::move(expr),
            op,
            std::move(right)
        );
    }

    return expr;
}

// multiplication -> unary (('*' | '/') unary)*

ExprPtr Parser::multiplication() {
    ExprPtr expr = unary();

    while (match({TokenType::Star, TokenType::Slash})) {
        TokenType op = previous().type;
        ExprPtr right = unary();

        expr = std::make_unique<BinaryExpr>(
            std::move(expr),
            op,
            std::move(right)
        );
    }

    return expr;
}

// unary -> '-' unary | primary

ExprPtr Parser::unary() {
    if (match({TokenType::Minus})) {
        TokenType op = previous().type;
        ExprPtr operand = unary();

        return std::make_unique<UnaryExpr>(
            op,
            std::move(operand)
        );
    }

    return primary();
}

// primary -> INTEGER | IDENTIFIER | '(' expression ')'

ExprPtr Parser::primary() {
    if (match({TokenType::Integer})) {
        const Token& token = previous();

        try {
            std::size_t parsed = 0;
            long long value = std::stoll(token.lexeme, &parsed);

            if (parsed != token.lexeme.size()) {
                throw errorAt(
                    token,
                    "Invalid integer literal"
                );
            }

            return std::make_unique<IntegerExpr>(
                static_cast<std::int64_t>(value)
            );
        } catch (const std::out_of_range&) {
            throw errorAt(
                token,
                "Integer literal is out of range"
            );
        } catch (const std::invalid_argument&) {
            throw errorAt(
                token,
                "Invalid integer literal"
            );
        }
    }

    if (match({TokenType::Identifier})) {
        return std::make_unique<VariableExpr>(
            previous().lexeme
        );
    }

    if (match({TokenType::LeftParen})) {
        ExprPtr expr = expression();

        consume(
            TokenType::RightParen,
            "Expected ')' after expression"
        );

        return expr;
    }

    if (check(TokenType::Invalid)) {
        throw errorAt(
            peek(),
            "Invalid character in expression"
        );
    }

    throw errorAt(
        peek(),
        "Expected an expression"
    );
}
