#include "parser.h"
#include <iostream>

std::vector<Command> Parser::parse(const std::vector<Token>& tokens) {
    std::vector<Command> commands; 
    root_.value = "Root";
    
    Node* currentNode = &root_;
    
    for (const auto& token : tokens) { 
        Command cmd; // Create a Command object
        if (token.type == TokenType::END) break;
        
        Node* newNode = new Node(); // Allocate new Node dynamically
        newNode->value = token.value; // Set value for the new Node
        currentNode->children.push_back(newNode); // Add newNode to currentNode's children

        if (token.type == TokenType::PIPE) { 
            // Handle pipe command
            cmd.executable = "|"; // Set executable for pipe command
            Node* pipeNode = new Node(); // Allocate new Node dynamically for pipe
            pipeNode->value = "|"; // Set value for the pipe Node
            pipeNode->children.push_back(currentNode); // Add currentNode to pipeNode's children
            currentNode = pipeNode; // Update currentNode to point to pipeNode
            root_.children.push_back(pipeNode); // Add pipeNode to root's children

        } else if (token.type == TokenType::REDIRECT_OUT || token.type == TokenType::REDIRECT_IN) {
            // Handle redirection
            Node* redirectNode = new Node(); // Allocate new Node dynamically for redirection
            redirectNode->value = token.value; // Set value for the redirect Node
            redirectNode->children.push_back(currentNode); // Add currentNode to redirectNode's children
            currentNode = redirectNode; // Update currentNode to point to redirectNode
            root_.children.push_back(redirectNode); // Add redirectNode to root's children

        } else { 
            // Add as child of current node
            cmd.executable = token.value; // Set executable for the command
            currentNode->children.push_back(newNode); // Add as child of current node
            commands.push_back(cmd); // Add the command to the vector
        }
    }
    return commands; // Return the vector of commands
}

void Parser::deleteNodes(Node* node) {
    for (auto child : node->children) {
        deleteNodes(child); // Recursively delete child nodes
        delete child; // Delete the current node
    }
}

Parser::~Parser() {
    deleteNodes(&root_); // Clean up the root node and its children
}

void Parser::printParseTreeHelper(Node* node, int level) {
    for (int i = 0; i < level; ++i) std::cout << "  ";
    std::cout << node->value << std::endl;
    
    for (const auto& child : node->children) {
        printParseTreeHelper(child, level + 1);
    }
}

void Parser::printParseTree() {
    printParseTreeHelper(&root_, 0);
}
