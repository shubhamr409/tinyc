
#pragma once

#include "token.hpp"

#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(std::string source);

    std::vector<Token> tokenize();

private:
    std::string source_;
    std::size_t position_ = 0;
    int line_ = 1;
    int column_ = 1;

    char peek() const;
    char peekNext() const;
    char advance();

    void skipWhitespaceAndComments();
};
