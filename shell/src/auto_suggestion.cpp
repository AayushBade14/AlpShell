#include "auto_suggestion.h"
#include "history.h"
#include <algorithm>
#include <readline/readline.h>

// Global state for suggestions
std::string current_suggestion;
bool suggestion_active = false;
static int history_index = 0;
static std::string original_input;

// Find best matching command from history
std::string findHistorySuggestion(const std::string &input)
{
    if (input.empty())
        return "";

    const auto &history = History::getHistory();
    for (auto it = history.rbegin(); it != history.rend(); ++it)
    {
        if (it->find(input) == 0 && *it != input)
        {
            return *it;
        }
    }
    return "";
}

// Display suggestion as ghost text
void displaySuggestion()
{
    if (!suggestion_active || current_suggestion.empty() || rl_end != rl_point)
        return;

    // Use Readline's internal mechanism for non-printing characters
    rl_save_prompt();
    rl_insert_text(current_suggestion.substr(rl_end).c_str());
    rl_redisplay();
    rl_restore_prompt();
}

// Accept current suggestion (right arrow key)
int acceptSuggestion(int count, int key)
{
    if (suggestion_active)
    {
        rl_replace_line(current_suggestion.c_str(), 0);
        rl_end = rl_point = current_suggestion.length();
        suggestion_active = false;
        return 0;
    }
    return rl_forward_char(count, key);
}

// Up arrow with ghost text support
int handleUpArrow(int count, int key)
{
    const auto &history = History::getHistory();
    if (history.empty())
        return 0;

    // First press - capture current line and find suggestion
    if (!suggestion_active)
    {
        original_input = rl_line_buffer;
        history_index = 0;

        // Show current input + suggestion from history
        std::string current_input = rl_line_buffer;
        current_suggestion = findHistorySuggestion(current_input);

        if (!current_suggestion.empty())
        {
            suggestion_active = true;
            displaySuggestion();
            return 0;
        }
    }

    // Continue navigating when multiple presses
    if (history_index < history.size())
    {
        history_index++;
        rl_replace_line(history[history.size() - history_index].c_str(), 0);
        rl_point = rl_end;
        suggestion_active = false;
    }

    return 0;
}

// Down arrow restores original input
int handleDownArrow(int count, int key)
{
    if (history_index > 0)
    {
        history_index--;

        if (history_index == 0)
        {
            rl_replace_line(original_input.c_str(), 0);
        }
        else
        {
            const auto &history = History::getHistory();
            rl_replace_line(history[history.size() - history_index].c_str(), 0);
        }

        rl_point = rl_end;
    }

    return 0;
}
