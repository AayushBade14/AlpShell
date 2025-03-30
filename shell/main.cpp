#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <csignal>
#include "lexer.h"
#include "parser.h"
#include "command.h"
#include "executor.h"
#include "job_control.h"
#include "history.h"

int main() {
    // Setup signal handler for job control.
    JobControl::setupSignalHandlers();

    std::string line;
    while (true) {
        std::cout << "alpShell> ";
        std::getline(std::cin, line);
        if (line.empty()) continue;

        // Save command to history.
        History::addHistory(line);

        // Built-in command: exit.
        if (line == "exit") break;

        // Built-in command: history.
        if (line == "history") {
            History::printHistory();
            continue;
        }

        // Built-in command: jobs.
        if (line == "jobs") {
            JobControl::listJobs();
            continue;
        }

        // Tokenize input.
        Lexer lexer;
        std::vector<Token> tokens = lexer.tokenize(line);

        // Parse tokens into command(s).
        Parser parser;
        // Assume parser.parse now returns a vector of Command objects.
        std::vector<Command> commands = parser.parse(tokens);

        if (commands.empty()) continue;

        // Check for background execution (command ending with '&').
        bool runInBackground = false;
        if (!commands[0].arguments.empty()) {
            std::string lastArg = commands[0].arguments.back();
            if (lastArg == "&") {
                runInBackground = true;
                commands[0].arguments.pop_back();  // Remove '&'
            }
        }

        // Execute the command.
        if (runInBackground) {
            pid_t pid = fork();
            if (pid == 0) { // Child process
                executeCommand(commands[0]);
                exit(0);
            } else if (pid > 0) { // Parent process
                JobControl::addJob(pid, line);
                std::cout << "Started background job [" << pid << "]" << std::endl;
            } else {
                std::cerr << "Fork failed." << std::endl;
            }
        } else {
            executeCommand(commands[0]);
        }
    }
    return 0;
}
