#include "lexer.h"

std::vector<Token> Lexer::tokenize(const std::string& input) {
    std::vector<Token> tokens;
    std::string tokenValue;
    
    for (char c : input) {
        if (c == ' ') {
            if (!tokenValue.empty()) {
                if (tokenValue == "|") {
                    tokens.push_back({TokenType::PIPE, tokenValue});
                } else if (tokenValue == ">") {
                    tokens.push_back({TokenType::REDIRECT_OUT, tokenValue});
                } else if (tokenValue == "<") {
                    tokens.push_back({TokenType::REDIRECT_IN, tokenValue});
                } else {
                    tokens.push_back({TokenType::IDENTIFIER, tokenValue});
                }
                tokenValue.clear();
            }
        } else if (c == '|') {
            if (!tokenValue.empty()) {
                tokens.push_back({TokenType::IDENTIFIER, tokenValue});
                tokenValue.clear();
            }
            tokens.push_back({TokenType::PIPE, std::string(1, c)});
        } else if (c == '>' || c == '<') {
            if (!tokenValue.empty()) {
                tokens.push_back({TokenType::IDENTIFIER, tokenValue});
                tokenValue.clear();
            }
            tokens.push_back({TokenType::REDIRECT_OUT, std::string(1, c)});
        } else {
            tokenValue += c;
        }
    }
    
    if (!tokenValue.empty()) {
        tokens.push_back({TokenType::IDENTIFIER, tokenValue});
    }
    
    tokens.push_back({TokenType::END, ""});
    
    return tokens;
}

