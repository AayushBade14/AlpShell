// #include "lexer.h"

// std::vector<Token> Lexer::tokenize(const std::string& input) {
//     std::vector<Token> tokens;
//     std::string tokenValue;
    
//     for (char c : input) {
//         if (c == ' ') {
//             if (!tokenValue.empty()) {
//                 if (tokenValue == "|") {
//                     tokens.push_back({TokenType::PIPE, tokenValue});
//                 } else if (tokenValue == ">") {
//                     tokens.push_back({TokenType::REDIRECT_OUT, tokenValue});
//                 } else if (tokenValue == "<") {
//                     tokens.push_back({TokenType::REDIRECT_IN, tokenValue});
//                 } else {
//                     tokens.push_back({TokenType::IDENTIFIER, tokenValue});
//                 }
//                 tokenValue.clear();
//             }
//         } else if (c == '|') {
//             if (!tokenValue.empty()) {
//                 tokens.push_back({TokenType::IDENTIFIER, tokenValue});
//                 tokenValue.clear();
//             }
//             tokens.push_back({TokenType::PIPE, std::string(1, c)});
//         } else if (c == '>' || c == '<') {
//             if (!tokenValue.empty()) {
//                 tokens.push_back({TokenType::IDENTIFIER, tokenValue});
//                 tokenValue.clear();
//             }
//             tokens.push_back({TokenType::REDIRECT_OUT, std::string(1, c)});
//         } else {
//             tokenValue += c;
//         }
//     }
    
//     if (!tokenValue.empty()) {
//         tokens.push_back({TokenType::IDENTIFIER, tokenValue});
//     }
    
//     tokens.push_back({TokenType::END, ""});
    
//     return tokens;
// }



#include "lexer.h"
#include <iostream> // For potential error messages

// Define states for the lexer
enum class LexerState {
    DEFAULT,     // Outside any quotes
    IN_SQUOTE,   // Inside single quotes ('')
    IN_DQUOTE    // Inside double quotes ("")
};

std::vector<Token> Lexer::tokenize(const std::string& input) {
    std::vector<Token> tokens;
    std::string currentTokenValue;
    LexerState currentState = LexerState::DEFAULT;

    auto emitToken = [&](TokenType type) {
        if (!currentTokenValue.empty() || type == TokenType::END) { // Allow empty value only for END
             // Special handling for operators if they weren't accumulated
             if (currentTokenValue == "|") type = TokenType::PIPE;
             else if (currentTokenValue == ">") type = TokenType::REDIRECT_OUT;
             else if (currentTokenValue == "<") type = TokenType::REDIRECT_IN;
             // Add more operator checks if needed (e.g., '>>', '&')

             tokens.push_back({type, currentTokenValue});
             currentTokenValue.clear();
        }
    };

    for (char c : input) {
        switch (currentState) {
            case LexerState::DEFAULT:
                if (c == '\'') {
                    // Start of single-quoted string. Emit previous token if any.
                    emitToken(TokenType::IDENTIFIER);
                    currentState = LexerState::IN_SQUOTE;
                    // Don't add the quote itself to the token value
                } else if (c == '"') {
                    // Start of double-quoted string. Emit previous token if any.
                    emitToken(TokenType::IDENTIFIER);
                    currentState = LexerState::IN_DQUOTE;
                    // Don't add the quote itself to the token value
                } else if (c == '|') {
                    emitToken(TokenType::IDENTIFIER); // Emit previous token
                    currentTokenValue += c; // Add pipe character
                    emitToken(TokenType::PIPE);    // Emit pipe token
                } else if (c == '>') {
                    emitToken(TokenType::IDENTIFIER);
                    currentTokenValue += c;
                    emitToken(TokenType::REDIRECT_OUT);
                } else if (c == '<') {
                    emitToken(TokenType::IDENTIFIER);
                    currentTokenValue += c;
                    emitToken(TokenType::REDIRECT_IN);
                } else if (isspace(c)) {
                    // Whitespace delimits tokens in default state
                    emitToken(TokenType::IDENTIFIER);
                } else {
                    // Regular character
                    currentTokenValue += c;
                }
                break;

            case LexerState::IN_SQUOTE:
                if (c == '\'') {
                    // End of single-quoted string
                    // Emit the accumulated content as a single identifier token
                    emitToken(TokenType::IDENTIFIER);
                    currentState = LexerState::DEFAULT;
                    // Don't add the closing quote to the token value
                } else {
                    // Add any character literally within single quotes
                    currentTokenValue += c;
                }
                break;

            case LexerState::IN_DQUOTE:
                 // Basic implementation: Treat everything literally except "
                 // TODO (Future): Handle escape sequences like \" , \\ , \$ here if needed.
                if (c == '"') {
                    // End of double-quoted string
                    emitToken(TokenType::IDENTIFIER);
                    currentState = LexerState::DEFAULT;
                    // Don't add the closing quote to the token value
                } else {
                    currentTokenValue += c;
                }
                break;
        }
    } // End of character loop

    // After loop, check for unterminated quotes
    if (currentState == LexerState::IN_SQUOTE || currentState == LexerState::IN_DQUOTE) {
        std::cerr << "alpShell: Lexer Error: Unterminated quote." << std::endl;
        tokens.clear(); // Clear potentially partial tokens on error
        // Return empty vector to signal error to the parser/main loop
        return tokens;
    }

    // Emit any remaining token accumulated in DEFAULT state
    emitToken(TokenType::IDENTIFIER);

    // Add the final END token
    tokens.push_back({TokenType::END, ""});

    return tokens;
}