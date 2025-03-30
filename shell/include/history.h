#ifndef HISTORY_H
#define HISTORY_H

#include <string>
#include <vector>

namespace History {
    // Adds a command to the history.
    void addHistory(const std::string &command);
    
    // Prints the history.
    void printHistory();
    
    // Returns the current history vector.
    const std::vector<std::string>& getHistory();
}

#endif // HISTORY_H
