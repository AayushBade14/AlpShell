#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <csignal>
#include <cstdlib> // For free()
#include <unistd.h> // For fork(), pid_t

// --- Readline Headers ---
#include <readline/readline.h>
#include <readline/history.h>
// -----------------------

#include "lexer.h"
#include "parser.h"
#include "command.h"
#include "executor.h" // Include for executePipeline
#include "job_control.h"
#include "history.h"

int main() {
    // Setup signal handler for job control.
    JobControl::setupSignalHandlers();

    std::string line;
    char* read_buffer = nullptr;

    while (true) {
        if (read_buffer) {
            free(read_buffer);
            read_buffer = nullptr;
        }

        read_buffer = readline("alpShell> ");

        if (read_buffer == nullptr) {
            std::cout << std::endl;
            break;
        }

        if (read_buffer && *read_buffer) {
            add_history(read_buffer);
        }

        line = read_buffer;
        if (line.empty()) {
            continue;
        }

        History::addHistory(line);

        if (line == "exit") break;
        if (line == "history") {
            History::printHistory();
            continue;
        }
        if (line == "jobs") {
            JobControl::listJobs();
            continue;
        }

        // Tokenize input.
        Lexer lexer;
        std::vector<Token> tokens = lexer.tokenize(line);

        // Parse tokens into command(s).
        Parser parser;
        std::vector<Command> commands = parser.parse(tokens);

        if (commands.empty()) {
            continue;
        }

        // --- Check for background execution (&) AFTER parsing ---
        // The '&' should ideally be handled by the parser as a separate signal,
        // but for now, let's check if the *original* line ended with it,
        // while also removing it if it was parsed as the last argument of the last command.
        bool runInBackground = false;
        std::string trimmed_line = line; // Work with a copy for checking '&'

        // Trim trailing whitespace from the original line copy
        size_t endpos = trimmed_line.find_last_not_of(" \t");
        if (std::string::npos != endpos) {
            trimmed_line = trimmed_line.substr(0, endpos + 1);
        }

        if (!trimmed_line.empty() && trimmed_line.back() == '&') {
            runInBackground = true;
            // Remove '&' from the command structure if it was parsed as an argument
            // of the *last* command. This relies on the parser correctly placing it.
            Command& last_cmd = commands.back();
            if (!last_cmd.arguments.empty() && last_cmd.arguments.back() == "&") {
                 last_cmd.arguments.pop_back();
            }
             // Handle case where executable itself might be mistaken for '&'
             else if (last_cmd.arguments.empty() && last_cmd.executable == "&") {
                 // Invalid command like "cmd | &" - parser should ideally catch this
                 std::cerr << "alpShell: syntax error near unexpected token `&'" << std::endl;
                 continue;
             }
        }


        // --- Execute the pipeline ---
        pid_t last_pid = executePipeline(commands, runInBackground);

        // --- Add job to job control if running in background ---
        if (runInBackground && last_pid > 0) {
             // Use the original command line string (before potential '&' removal from structure)
            JobControl::addJob(last_pid, line); // Store job using PID of last command
            std::cout << "Started background job [" << last_pid << "]" << std::endl; // Report using last PID
        } else if (runInBackground && last_pid == 0) {
             std::cerr << "Failed to start background job." << std::endl;
        }

    } // End while loop

    if (read_buffer) {
        free(read_buffer);
    }
    return 0;
}