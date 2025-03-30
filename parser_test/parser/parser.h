#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "lexer.h"

class Parser {
public:
    std::vector<Command> parse(const std::vector<Token>& tokens);
    void printParseTree();
private:
    struct Node {
        std::string value;
        std::vector<Node> children;
    };
    Node root_;
};

#endif
