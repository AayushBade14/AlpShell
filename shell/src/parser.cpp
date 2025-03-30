#include "parser.h"
#include <iostream>

void Parser::parse(const std::vector<Token>& tokens) {
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
            // Add as child of current node
            currentNode->children.push_back(newNode);
        }
    }
}

void Parser::printParseTree(Node* node, int level) {
    for (int i = 0; i < level; ++i) std::cout << "  ";
    std::cout << node->value << std::endl;
    
    for (const auto& child : node->children) {
        printParseTree(&child, level + 1);
    }
}

void Parser::printParseTree() {
    printParseTree(&root_, 0);
}

