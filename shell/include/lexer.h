#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>

enum class TokenType {
    IDENTIFIER,
    NUMBER,
    PIPE,
    REDIRECT_OUT,
    REDIRECT_IN,
    END
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
public:
    std::vector<Token> tokenize(const std::string& input);
};

#endif

