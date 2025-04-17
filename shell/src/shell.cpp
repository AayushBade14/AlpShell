#include "shell.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include "executor.h"
#include "history.h"
#include "job_control.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

void AlpShell::alp_loop() {
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
}

// loop with arguments
void AlpShell::alp_loop(char **flags){

}

// method for prompt
void AlpShell::alp_prompt(){

}

// method to run shell with flags
void AlpShell::run(char **flags){
  AlpShell::alp_loop(flags);
}

// method to run shell without flags
void AlpShell::run(){
  AlpShell::alp_loop();
}
