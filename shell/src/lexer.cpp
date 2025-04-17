// #include "lexer.h"
// #include <iostream> // For potential error messages
// #include <cctype>   // For isspace, isalnum, etc.

// enum class LexerState {
//     DEFAULT,
//     IN_SQUOTE,
//     IN_DQUOTE
// };

// std::vector<Token> Lexer::tokenize(const std::string& input) {

//     std::vector<Token> tokens;
//     std::string currentTokenValue;
//     LexerState currentState = LexerState::DEFAULT;
//     QuotingType currentQuoting = QuotingType::NONE;

//     auto emitToken = [&](TokenType type) {
//         bool canBeEmpty = (type == TokenType::END || type == TokenType::PIPE ||
//                            type == TokenType::REDIRECT_IN || type == TokenType::REDIRECT_OUT);

//         if (!currentTokenValue.empty() || canBeEmpty) {
//              if (type == TokenType::IDENTIFIER && !currentTokenValue.empty()) {
//                  if (currentTokenValue == "|") type = TokenType::PIPE;
//                  else if (currentTokenValue == ">") type = TokenType::REDIRECT_OUT;
//                  else if (currentTokenValue == "<") type = TokenType::REDIRECT_IN;
//              }

//             if (type != TokenType::IDENTIFIER) {
//                 currentQuoting = QuotingType::NONE;
//             }

//             Token newToken;
//             newToken.type = type;
//             newToken.value = currentTokenValue;
//             newToken.quoting = currentQuoting;
//             tokens.push_back(newToken);

//             currentTokenValue.clear();
//             currentQuoting = QuotingType::NONE;
//         }
//     };

//     for (char c : input) {
//         switch (currentState) {
//             case LexerState::DEFAULT:
//                 if (c == '\'') {
//                     emitToken(TokenType::IDENTIFIER); // Emit anything prior
//                     currentState = LexerState::IN_SQUOTE;
//                     currentQuoting = QuotingType::SINGLE; // Set quoting for the token *about* to be built
//                 } else if (c == '"') {
//                     emitToken(TokenType::IDENTIFIER);
//                     currentState = LexerState::IN_DQUOTE;
//                     currentQuoting = QuotingType::DOUBLE;
//                 } else if (c == '|') {
//                     emitToken(TokenType::IDENTIFIER);
//                     currentTokenValue += c;
//                     emitToken(TokenType::PIPE);
//                 } else if (c == '>') {
//                     emitToken(TokenType::IDENTIFIER);
//                     currentTokenValue += c;
//                     emitToken(TokenType::REDIRECT_OUT);
//                 } else if (c == '<') {
//                     emitToken(TokenType::IDENTIFIER);
//                     currentTokenValue += c;
//                     emitToken(TokenType::REDIRECT_IN);
//                 } else if (isspace(c)) {
//                     emitToken(TokenType::IDENTIFIER); // Emit previous token
//                 } else {
//                     // Part of an unquoted identifier
//                     currentQuoting = QuotingType::NONE; // Ensure it's marked as NONE
//                     currentTokenValue += c;
//                 }
//                 break;

//             case LexerState::IN_SQUOTE:
//                 if (c == '\'') {
//                     // End of single-quoted string. currentQuoting is already SINGLE.
//                     emitToken(TokenType::IDENTIFIER);
//                     currentState = LexerState::DEFAULT;
//                 } else {
//                     currentTokenValue += c;
//                 }
//                 break;

//             case LexerState::IN_DQUOTE:
//                 // Basic: No escape sequences handled yet
//                 if (c == '"') {
//                     // End of double-quoted string. currentQuoting is already DOUBLE.
//                     emitToken(TokenType::IDENTIFIER);
//                     currentState = LexerState::DEFAULT;
//                 } else {
//                     currentTokenValue += c;
//                 }
//                 break;
//         }
//     } // End of character loop


//     if (currentState != LexerState::DEFAULT) {
//         std::cerr << "alpShell: Lexer Error: Unterminated quote." << std::endl;
//         tokens.clear();
//         return tokens;
//     }

//     // Emit any remaining token accumulated in DEFAULT state
//     // emitToken(TokenType::IDENTIFIER);
//     emitToken(TokenType::IDENTIFIER);

//     Token endToken;
//     endToken.type = TokenType::END;
//     endToken.value = "";
//     endToken.quoting = QuotingType::NONE;
//     tokens.push_back(endToken);

//     return tokens;
// }

#include "lexer.h"
#include <iostream>
#include <cctype>

enum class LexerState {
    DEFAULT,
    IN_SQUOTE,
    IN_DQUOTE
};

std::vector<Token> Lexer::tokenize(const std::string& input) {
    std::vector<Token> tokens;
    std::string currentTokenValue;
    LexerState currentState = LexerState::DEFAULT;
    QuotingType currentQuoting = QuotingType::NONE;

    auto emitToken = [&](TokenType type) {
        bool canBeEmpty = (type == TokenType::END || type == TokenType::PIPE ||
                           type == TokenType::REDIRECT_IN || type == TokenType::REDIRECT_OUT ||
                           type == TokenType::REDIRECT_APPEND); // Add new type

        if (!currentTokenValue.empty() || canBeEmpty) {
            if (type == TokenType::IDENTIFIER && !currentTokenValue.empty()) {
                if (currentTokenValue == "|") type = TokenType::PIPE;
                else if (currentTokenValue == ">") type = TokenType::REDIRECT_OUT;
                else if (currentTokenValue == "<") type = TokenType::REDIRECT_IN;
                else if (currentTokenValue == ">>") type = TokenType::REDIRECT_APPEND; // Add check
            }

            if (type != TokenType::IDENTIFIER) {
                currentQuoting = QuotingType::NONE;
            }

            Token newToken;
            newToken.type = type;
            newToken.value = currentTokenValue;
            newToken.quoting = currentQuoting;
            tokens.push_back(newToken);

            currentTokenValue.clear();
            currentQuoting = QuotingType::NONE;
        }
    };

    // --- Use indexed loop for lookahead ---
    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];

        switch (currentState) {
            case LexerState::DEFAULT:
                if (c == '\'') {
                    emitToken(TokenType::IDENTIFIER);
                    currentState = LexerState::IN_SQUOTE;
                    currentQuoting = QuotingType::SINGLE;
                } else if (c == '"') {
                    emitToken(TokenType::IDENTIFIER);
                    currentState = LexerState::IN_DQUOTE;
                    currentQuoting = QuotingType::DOUBLE;
                } else if (c == '|') {
                    emitToken(TokenType::IDENTIFIER);
                    currentTokenValue += c;
                    emitToken(TokenType::PIPE);
                } else if (c == '>') {
                    emitToken(TokenType::IDENTIFIER); // Emit previous
                    // --- Lookahead for >> ---
                    if (i + 1 < input.length() && input[i + 1] == '>') {
                        currentTokenValue = ">>"; // Store ">>" as value
                        emitToken(TokenType::REDIRECT_APPEND);
                        i++; // Increment i to skip the second '>'
                    } else {
                        currentTokenValue = ">"; // Store ">" as value
                        emitToken(TokenType::REDIRECT_OUT);
                    }
                    // ------------------------
                } else if (c == '<') {
                    emitToken(TokenType::IDENTIFIER);
                    currentTokenValue += c;
                    emitToken(TokenType::REDIRECT_IN);
                } else if (isspace(c)) {
                    emitToken(TokenType::IDENTIFIER);
                } else {
                    currentQuoting = QuotingType::NONE;
                    currentTokenValue += c;
                }
                break;

            case LexerState::IN_SQUOTE:
                if (c == '\'') {
                    emitToken(TokenType::IDENTIFIER);
                    currentState = LexerState::DEFAULT;
                } else {
                    currentTokenValue += c;
                }
                break;

            case LexerState::IN_DQUOTE:
                if (c == '"') {
                    emitToken(TokenType::IDENTIFIER);
                    currentState = LexerState::DEFAULT;
                } else {
                    // TODO: Handle escapes like \"
                    currentTokenValue += c;
                }
                break;
        }
    } // End of character loop

    if (currentState != LexerState::DEFAULT) {
        std::cerr << "alpShell: Lexer Error: Unterminated quote." << std::endl;
        tokens.clear();
        return tokens;
    }

    emitToken(TokenType::IDENTIFIER);

    Token endToken;
    endToken.type = TokenType::END;
    endToken.value = "";
    endToken.quoting = QuotingType::NONE;
    tokens.push_back(endToken);

    return tokens;
}