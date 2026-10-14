
#pragma once

#include <string>

enum class TokenType {
    EndOfFile,
    Identifier,
    Integer,

    Let,
    Print,
    While,

    Plus,
    Minus,
    Star,
    Slash,
    Equal,          // Assignment operator: =
    EqualEqual,     // Equality comparison: ==
    BangEqual,      // Inequality comparison: !=
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    Semicolon,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,

    Invalid
};

inline std::string tokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::EndOfFile: return "EOF";
        case TokenType::Identifier: return "IDENTIFIER";
        case TokenType::Integer: return "INTEGER";

        case TokenType::Let: return "LET";
        case TokenType::Print: return "PRINT";
        case TokenType::While: return "WHILE";

        case TokenType::Plus: return "PLUS";
        case TokenType::Minus: return "MINUS";
        case TokenType::Star: return "STAR";
        case TokenType::Slash: return "SLASH";
        case TokenType::Equal: return "EQUAL";
        case TokenType::EqualEqual: return "EQUAL_EQUAL";
        case TokenType::BangEqual: return "BANG_EQUAL";
        case TokenType::Less: return "LESS";
        case TokenType::LessEqual: return "LESS_EQUAL";
        case TokenType::Greater: return "GREATER";
        case TokenType::GreaterEqual: return "GREATER_EQUAL";

        case TokenType::Semicolon: return "SEMICOLON";
        case TokenType::LeftParen: return "LEFT_PAREN";
        case TokenType::RightParen: return "RIGHT_PAREN";
        case TokenType::LeftBrace: return "LEFT_BRACE";
        case TokenType::RightBrace: return "RIGHT_BRACE";

        case TokenType::Invalid: return "INVALID";
    }

    return "UNKNOWN";
}

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};
