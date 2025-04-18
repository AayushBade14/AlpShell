// filepath: /home/laksh/repos/AlpsOS/shell/src/syntax_highlight.cpp
#include "lexer.h"
#include "syntax_highlight.h"
#include "prompt.h"
extern "C"
{
    #include <readline/readline.h>
    #include <readline/history.h>
}
#include <string>
#include <vector>
#include <cstdlib> // For free()

// Ensure non-printing chars are properly escaped
// Update color definitions with proper Readline escaping
#define ERROR_COLOR "\001\033[1;31m\002"  // Red (bold)
#define PROMPT_COLOR "\001\033[1;34m\002" // Blue (bold)
#define RESET_COLOR "\001\033[0m\002"

// Check for syntax errors in the input line
int check_syntax(const char *line)
{
    Lexer lexer;
    std::vector<Token> tokens = lexer.tokenize(line);

    // Check for multiple error conditions
    bool in_quote = false;
    bool has_pipe_error = false;
    bool has_redirect_error = false;

    for (const auto &token : tokens)
    {
        // Unterminated quotes
        if (token.quoting != QuotingType::NONE)
            in_quote = !in_quote;

        // Check for pipe errors
        if (token.type == TokenType::PIPE && tokens.size() == 1)
            has_pipe_error = true;

        // Check for invalid redirects
        if ((token.type == TokenType::REDIRECT_IN ||
             token.type == TokenType::REDIRECT_OUT) &&
            (&token == &tokens.back()))
            has_redirect_error = true;
    }

    return in_quote || has_pipe_error || has_redirect_error ||
           (tokens.empty() && !std::string(line).empty());
}

// Check if a character is within quotes
int is_quoted(char *text, int index) // Match readline's expected signature
{
    const std::string str(text); // Create const copy
    Lexer lexer;
    std::vector<Token> tokens = lexer.tokenize(str);

    for (const auto &token : tokens)
    {
        if ((token.quoting == QuotingType::SINGLE || token.quoting == QuotingType::DOUBLE) &&
            index >= token.start_pos && index <= token.end_pos)
        {
            return 1;
        }
    }
    return 0;
}

// Hook to redisplay the prompt with syntax highlighting
// filepath: /home/laksh/repos/AlpsOS/shell/src/syntax_highlight.cpp
extern "C" void redisplay_hook()
{
    char *line = rl_copy_text(0, rl_end);
    int syntax_error = check_syntax(line);
    free(line);

    std::string base_prompt = get_prompt();
    size_t dollar_pos = base_prompt.find("$");

    if (syntax_error && dollar_pos != std::string::npos)
    {
        std::string error_prompt =
            base_prompt.substr(0, dollar_pos) +
            ERROR_COLOR "$" RESET_COLOR " ";
        rl_set_prompt(error_prompt.c_str());
    }
    else
    {
        rl_set_prompt(base_prompt.c_str());
    }
    rl_redisplay();
}
