#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "command.h"
#include "lexer.h"

class Parser {
public:
    ~Parser(); // Destructor declaration
    std::vector<Command> parse(const std::vector<Token>& tokens);
    void printParseTree();
private:
    struct Node {
        std::string value;
        std::vector<Node*> children; // Store pointers to Node
    };
    Node root_;
    void deleteNodes(Node* node); // Added helper function declaration
public:
    void printParseTreeHelper(Node* node, int level); // Moved declaration
};

#endif
