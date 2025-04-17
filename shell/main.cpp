// // #include <iostream>
// // #include <sstream>
// // #include <vector>
// // #include <string>
// // #include <csignal>
// // #include <cstdlib> // For free()
// // #include <cstdio>
// // #include <unistd.h> // For fork(), pid_t

// // // --- Readline Headers ---
// // #include <readline/readline.h>
// // #include <readline/history.h>
// // // -----------------------

// // #include "lexer.h"
// // #include "parser.h"
// // #include "command.h"
// // #include "executor.h" // Include for executePipeline
// // #include "job_control.h"
// // #include "history.h"

// // int main() {
// //     // Setup signal handler for job control.
// //     JobControl::setupSignalHandlers();

// //     std::string line;
// //     char* read_buffer = nullptr;

// //     while (true) {
// //         if (read_buffer) {
// //             free(read_buffer);
// //             read_buffer = nullptr;
// //         }

// //         read_buffer = readline("alpShell> ");

// //         if (read_buffer == nullptr) {
// //             std::cout << std::endl;
// //             break;
// //         }

// //         if (read_buffer && *read_buffer) {
// //             add_history(read_buffer);
// //         }

// //         line = read_buffer;
// //         if (line.empty()) {
// //             continue;
// //         }

// //         History::addHistory(line);

// //         if (line == "exit") break;
// //         if (line == "history") {
// //             History::printHistory();
// //             continue;
// //         }
// //         if (line == "jobs") {
// //             JobControl::listJobs();
// //             continue;
// //         }

// //         // Tokenize input.
// //         Lexer lexer;
// //         std::vector<Token> tokens = lexer.tokenize(line);

// //         // Parse tokens into command(s).
// //         Parser parser;
// //         std::vector<Command> commands = parser.parse(tokens);

// //         if (commands.empty()) {
// //             continue;
// //         }

// //         // --- Check for background execution (&) AFTER parsing ---
// //         // The '&' should ideally be handled by the parser as a separate signal,
// //         // but for now, let's check if the *original* line ended with it,
// //         // while also removing it if it was parsed as the last argument of the last command.
// //         bool runInBackground = false;
// //         std::string trimmed_line = line; // Work with a copy for checking '&'

// //         // Trim trailing whitespace from the original line copy
// //         size_t endpos = trimmed_line.find_last_not_of(" \t");
// //         if (std::string::npos != endpos) {
// //             trimmed_line = trimmed_line.substr(0, endpos + 1);
// //         }

// //         if (!trimmed_line.empty() && trimmed_line.back() == '&') {
// //             runInBackground = true;
// //             // Remove '&' from the command structure if it was parsed as an argument
// //             // of the *last* command. This relies on the parser correctly placing it.
// //             Command& last_cmd = commands.back();
// //             if (!last_cmd.arguments.empty() && last_cmd.arguments.back() == "&") {
// //                  last_cmd.arguments.pop_back();
// //             }
// //              // Handle case where executable itself might be mistaken for '&'
// //              else if (last_cmd.arguments.empty() && last_cmd.executable == "&") {
// //                  // Invalid command like "cmd | &" - parser should ideally catch this
// //                  std::cerr << "alpShell: syntax error near unexpected token `&'" << std::endl;
// //                  continue;
// //              }
// //         }


// //         // --- Execute the pipeline ---
// //         pid_t last_pid = executePipeline(commands, runInBackground);

// //         // --- Add job to job control if running in background ---
// //         if (runInBackground && last_pid > 0) {
// //              // Use the original command line string (before potential '&' removal from structure)
// //             JobControl::addJob(last_pid, line); // Store job using PID of last command
// //             std::cout << "Started background job [" << last_pid << "]" << std::endl; // Report using last PID
// //         } else if (runInBackground && last_pid == 0) {
// //              std::cerr << "Failed to start background job." << std::endl;
// //         }

// //     } // End while loop

// //     if (read_buffer) {
// //         free(read_buffer);
// //     }
// //     return 0;
// // }


// #include <iostream>
// #include <sstream>
// #include <vector>
// #include <string>
// #include <csignal>
// #include <cstdlib> // For free(), getenv()
// #include <cstdio>  // For perror()
// #include <unistd.h> // For fork(), pid_t, chdir()

// // --- Readline Headers ---
// #include <readline/readline.h>
// #include <readline/history.h>
// // -----------------------

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
//     char* read_buffer = nullptr;

//     while (true) {
//         if (read_buffer) {
//             free(read_buffer);
//             read_buffer = nullptr;
//         }

//         read_buffer = readline("alpShell> ");

//         if (read_buffer == nullptr) {
//             std::cout << std::endl;
//             break;
//         }

//         if (read_buffer && *read_buffer) {
//             add_history(read_buffer);
//         }

//         line = read_buffer;
//         if (line.empty()) {
//             continue;
//         }

//         History::addHistory(line);

//         // --- Built-in command checks ---
//         if (line == "exit") break;

//         if (line == "history") {
//             History::printHistory();
//             continue; // Go to next prompt
//         }
//         if (line == "jobs") {
//             JobControl::listJobs();
//             continue; // Go to next prompt
//         }

//         // --- Tokenize and Parse ---
//         Lexer lexer;
//         std::vector<Token> tokens = lexer.tokenize(line);
//         Parser parser;
//         std::vector<Command> commands = parser.parse(tokens);

//         if (commands.empty()) {
//             continue; // Parsing failed or resulted in nothing
//         }

//         // --- Built-in command: cd ---
//         // Check if the *first* command is 'cd'. cd doesn't work in pipes meaningfully.
//         if (commands[0].executable == "cd") {
//             std::string target_dir;
//             if (commands[0].arguments.empty()) {
//                 // Case 1: cd (no arguments) -> Go HOME
//                 const char* home_dir = getenv("HOME");
//                 if (home_dir == nullptr) {
//                     std::cerr << "alpShell: cd: HOME not set" << std::endl;
//                     continue; // Error, go to next prompt
//                 }
//                 target_dir = home_dir;
//             } else if (commands[0].arguments.size() == 1) {
//                 // Case 2: cd <directory>
//                 target_dir = commands[0].arguments[0];
//             } else {
//                 // Case 3: cd <too many arguments>
//                 std::cerr << "alpShell: cd: too many arguments" << std::endl;
//                 continue; // Error, go to next prompt
//             }

//             // Attempt to change directory
//             if (chdir(target_dir.c_str()) != 0) {
//                 // chdir failed, print system error
//                 perror(("alpShell: cd: " + target_dir).c_str());
//             }
//             // Whether chdir succeeded or failed, we are done processing 'cd'
//             continue; // Go to the next prompt
//         }

//         // --- End of built-in checks ---


//         // --- Background Execution Check ---
//         bool runInBackground = false;
//         std::string trimmed_line = line;
//         size_t endpos = trimmed_line.find_last_not_of(" \t");
//         if (std::string::npos != endpos) {
//             trimmed_line = trimmed_line.substr(0, endpos + 1);
//         }
//         if (!trimmed_line.empty() && trimmed_line.back() == '&') {
//             runInBackground = true;
//             Command& last_cmd = commands.back();
//             if (!last_cmd.arguments.empty() && last_cmd.arguments.back() == "&") {
//                  last_cmd.arguments.pop_back();
//             } else if (last_cmd.arguments.empty() && last_cmd.executable == "&") {
//                  std::cerr << "alpShell: syntax error near unexpected token `&'" << std::endl;
//                  continue;
//             }
//         }


//         // --- Execute the pipeline ---
//         pid_t last_pid = executePipeline(commands, runInBackground);

//         // --- Add job to job control if running in background ---
//         if (runInBackground && last_pid > 0) {
//             JobControl::addJob(last_pid, line);
//             std::cout << "Started background job [" << last_pid << "]" << std::endl;
//         } else if (runInBackground && last_pid == 0) {
//              std::cerr << "Failed to start background job." << std::endl;
//         }

//     } // End while loop

//     if (read_buffer) {
//         free(read_buffer);
//     }
//     return 0;
// }

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <csignal>
#include <cstdlib> // For free(), getenv(), exit()
#include <cstdio>  // For perror()
#include <unistd.h> // For fork(), pid_t, chdir(), isatty()
#include <fstream> // For reading script files

// --- Readline Headers ---
#include <readline/readline.h>
#include <readline/history.h>
// -----------------------

#include "lexer.h"
#include "parser.h"
#include "command.h"
#include "executor.h"
#include "job_control.h"
#include "history.h"

// Function to process a single line of command input
// Returns true to continue, false to exit
bool process_line(const std::string& line, bool is_interactive) {
    // if (line.empty()) {
    //     return true; // Ignore empty lines
    // }

    std::string trimmed_line = line;

    size_t first_char = trimmed_line.find_first_not_of(" \t");
    if (first_char == std::string::npos) {
        return true; // Line is empty or only whitespace
    }
    trimmed_line = trimmed_line.substr(first_char);

    if (trimmed_line.empty() || trimmed_line[0] == '#') {
        return true; // Skip empty lines and comments
    }



    // Only add interactive commands to history
    if (is_interactive) {
        // Add to readline's history if it came from readline
        // (We already did this before calling process_line for interactive mode)
        // Add to our internal history
        History::addHistory(line);
    }

    // --- Built-in command checks ---
    if (line == "exit") {
        return false; // Signal loop to exit
    }

    // These built-ins might make less sense in non-interactive mode,
    // but we can leave them for now.
    if (is_interactive && line == "history") {
        History::printHistory();
        return true;
    }
    if (line == "jobs") { // jobs can be useful even in scripts sometimes
        JobControl::listJobs();
        return true;
    }

    // --- Tokenize and Parse ---
    Lexer lexer;
    std::vector<Token> tokens = lexer.tokenize(line);
    // Check for lexer errors (e.g., unterminated quote returns empty tokens)
    if (tokens.empty() && !line.empty()) { // Check !line.empty() to distinguish from truly empty input
        std::cerr << "alpShell: Error during tokenization." << std::endl;
        // Decide script behavior: continue or exit? Let's continue for now.
        return true; // Or return false if scripts should exit on lexer error
    }
    // Handle case where only END token is returned for empty valid input
    if (tokens.size() <= 1 && tokens[0].type == TokenType::END) {
         return true;
    }


    Parser parser;
    std::vector<Command> commands = parser.parse(tokens);

    if (commands.empty()) {
        // Parsing failed or resulted in nothing (e.g., comment lines if implemented)
        // Check if it was due to a parsing error message already printed by parser
        return true; // Continue processing script/interactive session
    }

    // --- Built-in command: cd ---
    if (commands[0].executable == "cd") {
        std::string target_dir;
        if (commands[0].arguments.empty()) {
            const char* home_dir = getenv("HOME");
            if (home_dir == nullptr) {
                std::cerr << "alpShell: cd: HOME not set" << std::endl;
                return true;
            }
            target_dir = home_dir;
        } else if (commands[0].arguments.size() == 1) {
            target_dir = commands[0].arguments[0];
        } else {
            std::cerr << "alpShell: cd: too many arguments" << std::endl;
            return true;
        }
        if (chdir(target_dir.c_str()) != 0) {
            perror(("alpShell: cd: " + target_dir).c_str());
        }
        return true; // Continue after cd attempt
    }


    // // --- Background Execution Check ---
    // bool runInBackground = false;
    // // Check the *original* line for '&' as the background check in main was simpler.
    // // A more robust solution involves the parser identifying '&' as a separate token/flag.
    // std::string trimmed_line = line;
    // size_t endpos = trimmed_line.find_last_not_of(" \t");
    // if (std::string::npos != endpos) {
    //     trimmed_line = trimmed_line.substr(0, endpos + 1);
    // }
    // if (!trimmed_line.empty() && trimmed_line.back() == '&') {
    //     runInBackground = true;
    //     // Only enable background execution if not explicitly disabled for scripts,
    //     // but standard shells allow background jobs in scripts.
    //     Command& last_cmd = commands.back();
    //     if (!last_cmd.arguments.empty() && last_cmd.arguments.back() == "&") {
    //          last_cmd.arguments.pop_back();
    //     } else if (last_cmd.arguments.empty() && last_cmd.executable == "&") {
    //          std::cerr << "alpShell: syntax error near unexpected token `&'" << std::endl;
    //          return true;
    //     }
    // }

    // --- Background Execution Check ---
    // Check the original line for '&'
    bool runInBackground = false;
    std::string original_trimmed = line; // Use original line for this check
    size_t endpos = original_trimmed.find_last_not_of(" \t");
    if (std::string::npos != endpos) {
        original_trimmed = original_trimmed.substr(0, endpos + 1);
    }
     if (!original_trimmed.empty() && original_trimmed.back() == '&') {
        runInBackground = true;
        Command& last_cmd = commands.back();
        if (!last_cmd.arguments.empty() && last_cmd.arguments.back() == "&") {
             last_cmd.arguments.pop_back();
        } else if (last_cmd.arguments.empty() && last_cmd.executable == "&") {
             std::cerr << "alpShell: syntax error near unexpected token `&'" << std::endl;
             return true;
        }
    }


    // --- Execute the pipeline ---
    pid_t last_pid = executePipeline(commands, runInBackground);


    // --- Add job to job control if running in background ---
    // Background jobs are usually allowed in scripts too
    if (runInBackground && last_pid > 0) {
        JobControl::addJob(last_pid, line);
        // Optionally suppress this message in non-interactive mode
        if (is_interactive) {
             std::cout << "Started background job [" << last_pid << "]" << std::endl;
        }
    } else if (runInBackground && last_pid == 0) {
         std::cerr << "Failed to start background job." << std::endl;
         // Decide script behavior: continue or exit?
         // return false; // Optionally exit script on background failure
    }

    // If a foreground command failed, should the script exit?
    // Standard shells often have `set -e` for this. We don't implement that yet.

    return true; // Continue processing
}


// --- Main Function ---
int main(int argc, char *argv[]) {
    // Setup signal handler for job control (needed in both modes)
    JobControl::setupSignalHandlers();

    bool is_interactive = false;
    std::istream* input_stream = &std::cin; // Default to stdin
    std::ifstream script_file;

    // Determine if running interactively or executing a script
    if (argc == 1) {
        // Check if stdin is connected to a terminal
        is_interactive = isatty(STDIN_FILENO);
    } else if (argc == 2) {
        // Attempt to open the script file
        script_file.open(argv[1]);
        if (!script_file.is_open()) {
            perror(("alpShell: Failed to open script file: " + std::string(argv[1])).c_str());
            return EXIT_FAILURE;
        }
        input_stream = &script_file; // Point input stream to the file
        is_interactive = false;
    } else {
        std::cerr << "Usage: " << argv[0] << " [script_file]" << std::endl;
        return EXIT_FAILURE;
    }

    // --- Main Processing Loop ---
    std::string line;
    char* read_buffer = nullptr; // Only for readline

    if (is_interactive) {
        // Interactive loop using readline
        while (true) {
            if (read_buffer) {
                free(read_buffer);
                read_buffer = nullptr;
            }
            read_buffer = readline("alpShell> ");
            if (read_buffer == nullptr) { // EOF (Ctrl+D)
                std::cout << std::endl;
                break;
            }
            if (read_buffer && *read_buffer) {
                 add_history(read_buffer); // Add to readline history BEFORE processing
            }
            line = read_buffer;

            if (!process_line(line, is_interactive)) {
                 break; // process_line returned false (e.g., exit command)
            }
        }
        if (read_buffer) free(read_buffer); // Final cleanup
    } else {
        // Non-interactive loop reading from file or redirected stdin
        while (std::getline(*input_stream, line)) {
             if (!process_line(line, is_interactive)) {
                 break; // process_line returned false (e.g., exit command)
            }
        }
         // Check for errors reading from the file stream if needed
         // if (input_stream->bad()) { ... }
    }

    return EXIT_SUCCESS;
}