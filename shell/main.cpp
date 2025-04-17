// // #include <iostream>
// // #include <unistd.h>
// // #include <sstream>
// // #include <vector>
// // #include <string>
// // #include <csignal>
// // #include "lexer.h"
// // #include "parser.h"
// // #include "command.h"
// // #include "executor.h"
// // #include "job_control.h"
// // #include "history.h"

// #include <iostream>
// #include <sstream>
// #include <vector>
// #include <string>
// #include <csignal>
// #include <cstdlib> // For free()
// #include <unistd.h>

// #include <readline/readline.h>
// #include <readline/history.h>

// #include "lexer.h"
// #include "parser.h"
// #include "command.h"
// #include "executor.h"
// #include "job_control.h"
// #include "history.h"

// int main() {
//     // Setup signal handler for job control.
//     JobControl::setupSignalHandlers();

//     std::string line;
//     while (true) {
//         std::cout << "alpShell> ";
//         std::getline(std::cin, line);
//         if (line.empty()) continue;

//         // Save command to history.
//         History::addHistory(line);

//         // Built-in command: exit.
//         if (line == "exit") break;

//         // Built-in command: history.
//         if (line == "history") {
//             History::printHistory();
//             continue;
//         }

//         // Built-in command: jobs.
//         if (line == "jobs") {
//             JobControl::listJobs();
//             continue;
//         }

//         // Tokenize input.
//         Lexer lexer;
//         std::vector<Token> tokens = lexer.tokenize(line);

//         // Parse tokens into command(s).
//         Parser parser;
//         // Assume parser.parse now returns a vector of Command objects.
//         std::vector<Command> commands = parser.parse(tokens);

//         if (commands.empty()) continue;

//         // Check for background execution (command ending with '&').
//         bool runInBackground = false;
//         if (!commands[0].arguments.empty()) {
//             std::string lastArg = commands[0].arguments.back();
//             if (lastArg == "&") {
//                 runInBackground = true;
//                 commands[0].arguments.pop_back();  // Remove '&'
//             }
//         }

//         // Execute the command.
//         if (runInBackground) {
//             pid_t pid = fork();
//             if (pid == 0) { // Child process
//                 executeCommand(commands[0]);
//                 exit(0);
//             } else if (pid > 0) { // Parent process
//                 JobControl::addJob(pid, line);
//                 std::cout << "Started background job [" << pid << "]" << std::endl;
//             } else {
//                 std::cerr << "Fork failed." << std::endl;
//             }
//         } else {
//             executeCommand(commands[0]);
//         }
//     }
//     return 0;
// }


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
#include "executor.h"
#include "job_control.h"
#include "history.h" // Keep for the 'history' built-in command

int main() {
    // Setup signal handler for job control.
    JobControl::setupSignalHandlers();

    std::string line; // Keep std::string for rest of the code
    char* read_buffer = nullptr; // Buffer for readline's return value

    while (true) {
        // Free the buffer from the previous iteration, if it exists
        if (read_buffer) {
            free(read_buffer);
            read_buffer = nullptr;
        }

        // Use readline to get input
        read_buffer = readline("alpShell> ");

        // Check for EOF (Ctrl+D) or error from readline
        if (read_buffer == nullptr) {
            std::cout << std::endl; // Print newline on EOF for cleaner exit
            break; // Exit loop
        }

        // If the line has content, add it to readline's history
        if (read_buffer && *read_buffer) {
            add_history(read_buffer);
        }

        // Convert C-style string from readline to std::string
        line = read_buffer;

        // Skip processing if the line is effectively empty after conversion
        if (line.empty()) {
            continue;
        }

        // --- Keep using your original History module for the 'history' command ---
        History::addHistory(line);
        // ------------------------------------------------------------------------


        // Built-in command: exit.
        if (line == "exit") break;

        // Built-in command: history.
        // This uses your internal History module.
        if (line == "history") {
            History::printHistory();
            continue;
        }

        // Built-in command: jobs.
        if (line == "jobs") {
            JobControl::listJobs();
            continue;
        }

        // --- Command processing logic ---

        // Tokenize input.
        Lexer lexer;
        std::vector<Token> tokens = lexer.tokenize(line);

        // Parse tokens into command(s).
        Parser parser;
        std::vector<Command> commands = parser.parse(tokens);

        // If parsing results in no commands (e.g., error or empty after parse)
        if (commands.empty()) {
            continue;
        }

        // Check for background execution (&) - Using the original logic based on commands[0]
        bool runInBackground = false;
        // Check if the first command has arguments before accessing back()
        if (!commands[0].arguments.empty()) {
            std::string lastArg = commands[0].arguments.back();
            if (lastArg == "&") {
                runInBackground = true;
                commands[0].arguments.pop_back(); // Remove '&' from the arguments

                // Handle case where command might become empty after removing '&' if it was the only arg
                // (The parser should ideally handle '&' as a separate token type)
            }
        }


        // Execute the command - Using the original logic (executes only commands[0])
        if (runInBackground) {
            pid_t pid = fork();
            if (pid == 0) { // Child process
                executeCommand(commands[0]);
                exit(0); // Exit child process after exec attempt
            } else if (pid > 0) { // Parent process
                // Use the original command line string for job reporting
                JobControl::addJob(pid, line);
                // Use the original background job message
                std::cout << "Started background job [" << pid << "]" << std::endl;
            } else { // Fork failed
                std::cerr << "Fork failed." << std::endl;
            }
        } else { // Foreground execution
            executeCommand(commands[0]); // Execute the first command and wait (implicit in executeCommand)
        }
    }

    // Cleanup readline buffer if loop exited while it held memory
    if (read_buffer) {
        free(read_buffer);
    }

    return 0;
}