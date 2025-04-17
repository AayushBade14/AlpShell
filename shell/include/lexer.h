// // // #ifndef LEXER_H
// // // #define LEXER_H

// // // #include <vector>
// // // #include <string>

// // // enum class TokenType {
// // //     IDENTIFIER,
// // //     NUMBER,
// // //     PIPE,
// // //     REDIRECT_OUT,
// // //     REDIRECT_IN,
// // //     END
// // // };

// // // struct Token {
// // //     TokenType type;
// // //     std::string value;
// // // };

// // // class Lexer {
// // // public:
// // //     std::vector<Token> tokenize(const std::string& input);
// // // };

// // // #endif

// // #ifndef LEXER_H
// // #define LEXER_H

// // #include <vector>
// // #include <string>

// // enum class TokenType {
// //     IDENTIFIER,
// //     NUMBER, // Still unused? Consider removing if not planned.
// //     PIPE,
// //     REDIRECT_OUT,
// //     REDIRECT_IN,
// //     END
// //     // Add other operators like >>, & if needed
// // };

// // // Define how a token was quoted (or not)
// // enum class QuotingType {
// //     NONE,         // No quotes
// //     SINGLE,       // Enclosed in '...'
// //     DOUBLE        // Enclosed in "..."
// // };

// // struct Token {
// //     TokenType type;
// //     std::string value;
// //     QuotingType quoting = QuotingType::NONE; // Default to no quoting
// // };

// // class Lexer {
// // public:
// //     std::vector<Token> tokenize(const std::string& input);
// // };

// // #endif // LEXER_H

#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>

enum class TokenType {
    IDENTIFIER,
    NUMBER,
    PIPE,
    REDIRECT_OUT,   // For >
    REDIRECT_IN,    // For <
    REDIRECT_APPEND,// For >>  <--- ADD THIS
    END
    // Add other operators like & if needed
};

// Define how a token was quoted (or not)
enum class QuotingType {
    NONE,         // No quotes
    SINGLE,       // Enclosed in '...'
    DOUBLE        // Enclosed in "..."
};

enum class LexerState
{
    DEFAULT,
    IN_SQUOTE,
    IN_DQUOTE
};

struct Token
{
    TokenType type;
    std::string value;
    QuotingType quoting = QuotingType::NONE;
    size_t start_pos = 0; // Add position tracking
    size_t end_pos = 0;   // Add position tracking
};

class Lexer {
public:
    std::vector<Token> tokenize(const std::string& input);
};

#endif // LEXER_H