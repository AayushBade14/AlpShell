#include "parser.h"
#include "command.h"
#include <iostream>

std::vector<Command> Parser::parse(const std::vector<Token>& tokens) {
    std::vector<Command> commands; 
    root_.value = "Root";
    
    Node* currentNode = &root_;
    
    for (const auto& token : tokens) {
        if (token.type == TokenType::END) break;
        
        Node newNode;
        newNode.value = token.value;
        
        if (token.type == TokenType::PIPE) {
            // Handle pipe command
            Node pipeNode;
            pipeNode.value = "|";
            pipeNode.children.push_back(*currentNode);
            currentNode = &pipeNode;
            root_.children.push_back(pipeNode);
        } else if (token.type == TokenType::REDIRECT_OUT || token.type == TokenType::REDIRECT_IN) {
            // Handle redirection
            Node redirectNode;
            redirectNode.value = token.value;
            redirectNode.children.push_back(*currentNode);
            currentNode = &redirectNode;
            root_.children.push_back(redirectNode);
        } else {
            // Create a new Command and populate it
            Command cmd;
            cmd.executable = token.value; // Assuming token.value is the command
            commands.push_back(cmd);
    }
    return commands; // Return the vector of commands
