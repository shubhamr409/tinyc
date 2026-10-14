
#include "lexer.hpp"

#include <cctype>
#include <utility>

Lexer::Lexer(std::string source)
    : source_(std::move(source)) {}

char Lexer::peek() const {
    if (position_ >= source_.size()) {
        return '\0';
    }

    return source_[position_];
}

char Lexer::peekNext() const {
    if (position_ + 1 >= source_.size()) {
        return '\0';
    }

    return source_[position_ + 1];
}

char Lexer::advance() {
    char current = peek();

    if (current == '\0') {
        return current;
    }

    ++position_;

    if (current == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }

    return current;
}

void Lexer::skipWhitespaceAndComments() {
    while (true) {
        while (std::isspace(
            static_cast<unsigned char>(peek()))) {
            advance();
        }

        if (peek() == '/' && peekNext() == '/') {
            while (peek() != '\n' && peek() != '\0') {
                advance();
            }

            continue;
        }

        break;
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        skipWhitespaceAndComments();

        const int startLine = line_;
        const int startColumn = column_;
        const std::size_t start = position_;
        const char current = peek();

        if (current == '\0') {
            tokens.push_back({
                TokenType::EndOfFile,
                "",
                line_,
                column_
            });

            break;
        }

        // Identifiers and keywords.
        if (std::isalpha(
                static_cast<unsigned char>(current)) ||
            current == '_') {

            advance();

            while (std::isalnum(
                       static_cast<unsigned char>(peek())) ||
                   peek() == '_') {
                advance();
            }

            std::string text =
                source_.substr(start, position_ - start);

            TokenType type = TokenType::Identifier;

            if (text == "let") {
                type = TokenType::Let;
            } else if (text == "print") {
                type = TokenType::Print;
            } else if (text == "while") {
                type = TokenType::While;
            }

            tokens.push_back({
                type,
                text,
                startLine,
                startColumn
            });

            continue;
        }

        // Integer literals.
        if (std::isdigit(
                static_cast<unsigned char>(current))) {

            advance();

            while (std::isdigit(
                       static_cast<unsigned char>(peek()))) {
                advance();
            }

            tokens.push_back({
                TokenType::Integer,
                source_.substr(start, position_ - start),
                startLine,
                startColumn
            });

            continue;
        }

        // Operators and delimiters. The two-character operators must be
        // recognized before their one-character prefixes.
        advance();

        TokenType type = TokenType::Invalid;
        std::string lexeme(1, current);

        switch (current) {
            case '+': type = TokenType::Plus; break;
            case '-': type = TokenType::Minus; break;
            case '*': type = TokenType::Star; break;
            case '/': type = TokenType::Slash; break;

            case '=':
                if (peek() == '=') {
                    advance();
                    type = TokenType::EqualEqual;
                    lexeme = "==";
                } else {
                    type = TokenType::Equal;
                }
                break;

            case '!':
                if (peek() == '=') {
                    advance();
                    type = TokenType::BangEqual;
                    lexeme = "!=";
                }
                break;

            case '<':
                if (peek() == '=') {
                    advance();
                    type = TokenType::LessEqual;
                    lexeme = "<=";
                } else {
                    type = TokenType::Less;
                }
                break;

            case '>':
                if (peek() == '=') {
                    advance();
                    type = TokenType::GreaterEqual;
                    lexeme = ">=";
                } else {
                    type = TokenType::Greater;
                }
                break;

            case ';': type = TokenType::Semicolon; break;
            case '(': type = TokenType::LeftParen; break;
            case ')': type = TokenType::RightParen; break;
            case '{': type = TokenType::LeftBrace; break;
            case '}': type = TokenType::RightBrace; break;

            default:
                break;
        }

        tokens.push_back({
            type,
            lexeme,
            startLine,
            startColumn
        });
    }

    return tokens;
}
